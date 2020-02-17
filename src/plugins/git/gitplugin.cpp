#include <coreplugin/messagebox.h>
#include <aggregation/aggregate.h>
#include <utils/fancylineedit.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
class GitLogEditorWidget : public QWidget
{
    Q_DECLARE_TR_FUNCTIONS(Git::Internal::GitLogEditorWidget);
public:
    GitLogEditorWidget()
    {
        auto gitEditor = new GitEditorWidget;
        auto vlayout = new QVBoxLayout;
        auto hlayout = new QHBoxLayout;
        vlayout->setSpacing(0);
        vlayout->setContentsMargins(0, 0, 0, 0);
        auto grepLineEdit = addLineEdit(tr("Filter by message"),
                                        tr("Filter log entries by text in the commit message."));
        auto pickaxeLineEdit = addLineEdit(tr("Filter by content"),
                                           tr("Filter log entries by added or removed string."));
        hlayout->setSpacing(20);
        hlayout->setContentsMargins(0, 0, 0, 0);
        hlayout->addWidget(new QLabel(tr("Filter:")));
        hlayout->addWidget(grepLineEdit);
        hlayout->addWidget(pickaxeLineEdit);
        hlayout->addStretch();
        vlayout->addLayout(hlayout);
        vlayout->addWidget(gitEditor);
        setLayout(vlayout);
        gitEditor->setGrepLineEdit(grepLineEdit);
        gitEditor->setPickaxeLineEdit(pickaxeLineEdit);

        auto textAgg = Aggregation::Aggregate::parentAggregate(gitEditor);
        auto agg = textAgg ? textAgg : new Aggregation::Aggregate;
        agg->add(this);
        agg->add(gitEditor);
        setFocusProxy(gitEditor);
    }

private:
    FancyLineEdit *addLineEdit(const QString &placeholder, const QString &tooltip)
    {
        auto lineEdit = new FancyLineEdit;
        lineEdit->setFiltering(true);
        lineEdit->setToolTip(tooltip);
        lineEdit->setPlaceholderText(placeholder);
        lineEdit->setMaximumWidth(200);
        return lineEdit;
    }
};

        [] { return new GitLogEditorWidget; },

    connect(VcsOutputWindow::instance(), &VcsOutputWindow::referenceClicked,
            this, [this](const QString &name) {
        const VcsBasePluginState state = currentState();
        QTC_ASSERT(state.hasTopLevel(), return);
        m_gitClient.show(state.topLevel(), name);
    });

    VcsBaseEditorWidget::testDiffFileResolving(dd->commitTextEditorFactory);
    VcsBaseEditorWidget::testLogResolving(dd->logEditorFactory, data,