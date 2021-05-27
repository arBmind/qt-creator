/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "filesystemfilter.h"

#include "basefilefilter.h"
#include "locatorwidget.h"

#include <coreplugin/coreconstants.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/icore.h>
#include <coreplugin/idocument.h>
#include <coreplugin/vcsmanager.h>
#include <utils/checkablemessagebox.h>
#include <utils/fileutils.h>
#include <utils/link.h>

#include <QDir>
#include <QJsonObject>
#include <QPushButton>
#include <QRegularExpression>

using namespace Core;
using namespace Core::Internal;
using namespace Utils;

ILocatorFilter::MatchLevel FileSystemFilter::matchLevelFor(const QRegularExpressionMatch &match,
                                                           const QString &matchText)
{
    const int consecutivePos = match.capturedStart(1);
    if (consecutivePos == 0)
        return MatchLevel::Best;
    if (consecutivePos > 0) {
        const QChar prevChar = matchText.at(consecutivePos - 1);
        if (prevChar == '_' || prevChar == '.')
            return MatchLevel::Better;
    }
    if (match.capturedStart() == 0)
        return MatchLevel::Good;
    return MatchLevel::Normal;
}

FileSystemFilter::FileSystemFilter()
{
    setId("Files in file system");
    setDisplayName(tr("Files in File System"));
    setDescription(tr("Opens a file given by a relative path to the current document, or absolute "
                      "path. \"~\" refers to your home directory. You have the option to create a "
                      "file if it does not exist yet."));
    setDefaultShortcutString("f");
    setDefaultIncludedByDefault(false);
}

void FileSystemFilter::prepareSearch(const QString &entry)
{
    Q_UNUSED(entry)
    m_currentDocumentDirectory = DocumentManager::fileDialogInitialDirectory();
    m_currentIncludeHidden = m_includeHidden;
}

QList<LocatorFilterEntry> FileSystemFilter::matchesFor(QFutureInterface<LocatorFilterEntry> &future,
                                                       const QString &entry)
{
    QList<LocatorFilterEntry> entries[int(MatchLevel::Count)];
    const QFileInfo entryInfo(entry);
    const QString entryFileName = entryInfo.fileName();
    QString directory = entryInfo.path();
    if (entryInfo.isRelative()) {
        if (entryInfo.filePath().startsWith("~/"))
            directory.replace(0, 1, QDir::homePath());
        else if (!m_currentDocumentDirectory.isEmpty())
            directory.prepend(m_currentDocumentDirectory + "/");
    }
    const QDir dirInfo(directory);
    QDir::Filters dirFilter = QDir::Dirs|QDir::Drives|QDir::NoDot|QDir::NoDotDot;
    QDir::Filters fileFilter = QDir::Files;
    if (m_currentIncludeHidden) {
        dirFilter |= QDir::Hidden;
        fileFilter |= QDir::Hidden;
    }
    // use only 'name' for case sensitivity decision, because we need to make the path
    // match the case on the file system for case-sensitive file systems
    const Qt::CaseSensitivity caseSensitivity_ = caseSensitivity(entryFileName);
    const QStringList dirs = QStringList("..")
            + dirInfo.entryList(dirFilter, QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);
    const QStringList files = dirInfo.entryList(fileFilter,
                                                QDir::Name|QDir::IgnoreCase|QDir::LocaleAware);

    QRegularExpression regExp = createRegExp(entryFileName, caseSensitivity_);
    if (!regExp.isValid())
        return {};

    for (const QString &dir : dirs) {
        if (future.isCanceled())
            break;

        const QRegularExpressionMatch match = regExp.match(dir);
        if (match.hasMatch()) {
            const MatchLevel level = matchLevelFor(match, dir);
            const QString fullPath = dirInfo.filePath(dir);
            LocatorFilterEntry filterEntry(this, dir, QVariant());
            filterEntry.filePath = FilePath::fromString(fullPath);
            filterEntry.highlightInfo = highlightInfo(match);

            entries[int(level)].append(filterEntry);
        }
    }
    // file names can match with +linenumber or :linenumber
    QString postfix;
    Link link = Link::fromString(entryFileName, true, &postfix);
    regExp = createRegExp(link.targetFilePath.toString(), caseSensitivity_);
    if (!regExp.isValid())
        return {};
    for (const QString &file : files) {
        if (future.isCanceled())
            break;

        const QRegularExpressionMatch match = regExp.match(file);
        if (match.hasMatch()) {
            const MatchLevel level = matchLevelFor(match, file);
            const QString fullPath = dirInfo.filePath(file);
            LocatorFilterEntry filterEntry(this, file, QString(fullPath + postfix));
            filterEntry.filePath = FilePath::fromString(fullPath);
            filterEntry.highlightInfo = highlightInfo(match);

            entries[int(level)].append(filterEntry);
        }
    }

    // "create and open" functionality
    const QString fullFilePath = dirInfo.filePath(link.targetFilePath.fileName());
    const bool containsWildcard = entry.contains('?') || entry.contains('*');
    if (!containsWildcard && !QFileInfo::exists(fullFilePath) && dirInfo.exists()) {
        LocatorFilterEntry createAndOpen(this, tr("Create and Open \"%1\"").arg(entry), fullFilePath);
        createAndOpen.extraInfo = Utils::FilePath::fromString(dirInfo.absolutePath()).shortNativePath();
        entries[int(MatchLevel::Normal)].append(createAndOpen);
    }

    return std::accumulate(std::begin(entries), std::end(entries), QList<LocatorFilterEntry>());
}

const char kAlwaysCreate[] = "Locator/FileSystemFilter/AlwaysCreate";

void FileSystemFilter::accept(LocatorFilterEntry selection,
                              QString *newText,
                              int *selectionStart,
                              int *selectionLength) const
{
    Q_UNUSED(selectionLength)
    QFileInfo info = selection.filePath.toFileInfo();
    if (info.isDir()) {
        const QString value = shortcutString() + ' '
                + QDir::toNativeSeparators(info.absoluteFilePath() + '/');
        *newText = value;
        *selectionStart = value.length();
    } else {
        // Don't block locator filter execution with dialog
        QMetaObject::invokeMethod(EditorManager::instance(), [info, selection] {
            const QString targetFile = selection.internalData.toString();
            if (!info.exists()) {
                if (Utils::CheckableMessageBox::shouldAskAgain(ICore::settings(), kAlwaysCreate)) {
                    Utils::CheckableMessageBox messageBox(ICore::dialogParent());
                    messageBox.setWindowTitle(tr("Create File"));
                    messageBox.setIcon(QMessageBox::Question);
                    messageBox.setText(
                        tr("Create \"%1\"?")
                            .arg(Utils::FilePath::fromString(targetFile).shortNativePath()));
                    messageBox.setCheckBoxVisible(true);
                    messageBox.setCheckBoxText(tr("Always create"));
                    messageBox.setChecked(false);
                    messageBox.setStandardButtons(QDialogButtonBox::Cancel);
                    QPushButton *createButton = messageBox.addButton(tr("Create"),
                                                                     QDialogButtonBox::AcceptRole);
                    messageBox.setDefaultButton(QDialogButtonBox::Cancel);
                    messageBox.exec();
                    if (messageBox.clickedButton() != createButton)
                        return;
                    if (messageBox.isChecked())
                        Utils::CheckableMessageBox::doNotAskAgain(ICore::settings(), kAlwaysCreate);
                }
                QFile file(targetFile);
                file.open(QFile::WriteOnly);
                file.close();
                VcsManager::promptToAdd(QFileInfo(targetFile).absolutePath(), { targetFile });
            }
            BaseFileFilter::openEditorAt(selection);
        }, Qt::QueuedConnection);
    }
}

bool FileSystemFilter::openConfigDialog(QWidget *parent, bool &needsRefresh)
{
    Q_UNUSED(needsRefresh)
    Ui::FileSystemFilterOptions ui;
    QDialog dialog(parent);
    ui.setupUi(&dialog);
    dialog.setWindowTitle(ILocatorFilter::msgConfigureDialogTitle());
    ui.prefixLabel->setText(ILocatorFilter::msgPrefixLabel());
    ui.prefixLabel->setToolTip(ILocatorFilter::msgPrefixToolTip());
    ui.includeByDefault->setText(msgIncludeByDefault());
    ui.includeByDefault->setToolTip(msgIncludeByDefaultToolTip());
    ui.hiddenFilesFlag->setChecked(m_includeHidden);
    ui.includeByDefault->setChecked(isIncludedByDefault());
    ui.shortcutEdit->setText(shortcutString());

    if (dialog.exec() == QDialog::Accepted) {
        m_includeHidden = ui.hiddenFilesFlag->isChecked();
        setShortcutString(ui.shortcutEdit->text().trimmed());
        setIncludedByDefault(ui.includeByDefault->isChecked());
        return true;
    }
    return false;
}

const char kIncludeHiddenKey[] = "includeHidden";

void FileSystemFilter::saveState(QJsonObject &object) const
{
    if (m_includeHidden != kIncludeHiddenDefault)
        object.insert(kIncludeHiddenKey, m_includeHidden);
}

void FileSystemFilter::restoreState(const QJsonObject &object)
{
    m_currentIncludeHidden = object.value(kIncludeHiddenKey).toBool(kIncludeHiddenDefault);
}

void FileSystemFilter::restoreState(const QByteArray &state)
{
    if (isOldSetting(state)) {
        // TODO read old settings, remove some time after Qt Creator 4.15
        QDataStream in(state);
        in >> m_includeHidden;

        // An attempt to prevent setting this on old configuration
        if (!in.atEnd()) {
            QString shortcut;
            bool defaultFilter;
            in >> shortcut;
            in >> defaultFilter;
            setShortcutString(shortcut);
            setIncludedByDefault(defaultFilter);
        }
    } else {
        ILocatorFilter::restoreState(state);
    }
}
