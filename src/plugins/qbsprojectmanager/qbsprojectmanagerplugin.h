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

#pragma once

#include <extensionsystem/iplugin.h>
#include <utils/id.h>
#include <utils/parameteraction.h>

namespace ProjectExplorer { class Target; }

namespace QbsProjectManager {
namespace Internal {

class QbsProject;
class QbsProjectManagerPluginPrivate;

class QbsProjectManagerPlugin final : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "QbsProjectManager.json")

public:
    static void buildNamedProduct(QbsProject *project, const QString &product);

private:
    ~QbsProjectManagerPlugin() final;

    bool initialize(const QStringList &arguments, QString *errorMessage) final;

    void targetWasAdded(ProjectExplorer::Target *target);
    void projectChanged();

    void generateVs2019Project();
    void debugWithVs2019Project();
    void buildFileContextMenu();
    void buildFile();
    void buildProductContextMenu();
    void cleanProductContextMenu();
    void rebuildProductContextMenu();
    void runStepsForProductContextMenu(const QList<Utils::Id> &stepTypes);
    void buildProduct();
    void cleanProduct();
    void rebuildProduct();
    void runStepsForProduct(const QList<Utils::Id> &stepTypes);
    void buildSubprojectContextMenu();
    void cleanSubprojectContextMenu();
    void rebuildSubprojectContextMenu();
    void runStepsForSubprojectContextMenu(const QList<Utils::Id> &stepTypes);

    void reparseSelectedProject();
    void reparseCurrentProject();
    void reparseProject(QbsProject *project);

    void updateContextActions();
    void updateReparseQbsAction();
    void updateBuildActions();

    void buildFiles(QbsProject *project, const QStringList &files,
                    const QStringList &activeFileTags);
    void buildSingleFile(QbsProject *project, const QString &file);

    static void runStepsForProducts(QbsProject *project,
                                    const QStringList &products,
                                    const QList<Utils::Id> &stepTypes);

    QbsProjectManagerPluginPrivate *d = nullptr;
    QAction *m_reparseQbs = nullptr;
    QAction *m_reparseQbsCtx = nullptr;
    QAction *m_buildFileCtx = nullptr;
    QAction *m_buildProductCtx = nullptr;
    QAction *m_cleanProductCtx = nullptr;
    QAction *m_rebuildProductCtx = nullptr;
    QAction *m_buildSubprojectCtx = nullptr;
    QAction *m_cleanSubprojectCtx = nullptr;
    QAction *m_rebuildSubprojectCtx = nullptr;
    QAction *m_menuAction = nullptr;
    QAction *m_generateVs2019Ctx = nullptr;
    QAction *m_debugWithVs2019Ctx = nullptr;
    Utils::ParameterAction *m_buildFile = nullptr;
    Utils::ParameterAction *m_buildProduct = nullptr;
    Utils::ParameterAction *m_cleanProduct = nullptr;
    Utils::ParameterAction *m_rebuildProduct = nullptr;
};

} // namespace Internal
} // namespace QbsProjectManager
