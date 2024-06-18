// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "findtoolwindow.h"
#include "searchresultwindow.h"
#include "util/searchreplaceworker.h"

#include "services/project/projectservice.h"
#include "services/editor/editorservice.h"

#include <DLineEdit>
#include <DLabel>
#include <DComboBox>
#include <DPushButton>
#include <DStackedWidget>
#include <DScrollArea>
#include <DFrame>
#include <DDialog>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QProcess>
#include <QFormLayout>
#include <QThread>

using namespace dpfservice;
DWIDGET_USE_NAMESPACE
class FindToolWindowPrivate : public QObject
{
public:
    explicit FindToolWindowPrivate(FindToolWindow *qq);
    ~FindToolWindowPrivate();

    void initUI();
    void initConnect();
    void initWorker();

    QWidget *createSearchParamWidget();
    QWidget *createSearchResultWidget();

    bool checkSelectedScopeValid();
    bool setBaseParams(BaseParams *params, bool onlyOpened = false);
    void showMessage(const QString &message);

public:
    FindToolWindow *q;
    DStackedWidget *stackedWidget { nullptr };
    SearchResultWindow *searchResultWindow { nullptr };
    QSharedPointer<SearchReplaceWorker> searchReplaceWorker { nullptr };
    QThread thread;

    DPushButton *searchBtn { nullptr };
    DPushButton *replaceBtn { nullptr };
    DComboBox *scopeComboBox { nullptr };
    DLineEdit *searchLineEdit { nullptr };
    DLineEdit *patternLineEdit { nullptr };
    DLineEdit *expatternLineEdit { nullptr };
    DPushButton *caseSensitiveCheckBtn { nullptr };
    DPushButton *wholeWordsCheckBtn { nullptr };

    friend class FindToolWindow;
};

FindToolWindowPrivate::FindToolWindowPrivate(FindToolWindow *qq)
    : q(qq)
{
}

FindToolWindowPrivate::~FindToolWindowPrivate()
{
    searchReplaceWorker->stop();
    thread.quit();
    thread.wait();
}

void FindToolWindowPrivate::initUI()
{
    DFrame *mainPaneFrame = new DFrame(q);
    DStyle::setFrameRadius(mainPaneFrame, 0);
    stackedWidget = new DStackedWidget();
    QHBoxLayout *hLayout = new QHBoxLayout(q);
    hLayout->setContentsMargins(0, 0, 0, 0);

    DScrollArea *scrollArea = new DScrollArea(mainPaneFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(stackedWidget);
    scrollArea->setLineWidth(0);

    mainPaneFrame->setLineWidth(0);
    hLayout->addWidget(mainPaneFrame);
    QHBoxLayout *scrollLayout = new QHBoxLayout();
    scrollLayout->addWidget(scrollArea);
    mainPaneFrame->setLayout(scrollLayout);

    stackedWidget->addWidget(createSearchParamWidget());
    stackedWidget->addWidget(createSearchResultWidget());
    stackedWidget->setCurrentIndex(0);
}

void FindToolWindowPrivate::initConnect()
{
    connect(searchBtn, &QAbstractButton::clicked, q, &FindToolWindow::search);
    connect(replaceBtn, &QAbstractButton::clicked, q, &FindToolWindow::replace);

    connect(searchResultWindow, &SearchResultWindow::reqBack, q, &FindToolWindow::switchSearchParamWidget);
    connect(searchResultWindow, &SearchResultWindow::reqReplace, q, &FindToolWindow::handleReplace);
}

void FindToolWindowPrivate::initWorker()
{
    searchReplaceWorker.reset(new SearchReplaceWorker);
    connect(searchReplaceWorker.data(), &SearchReplaceWorker::matched, q, &FindToolWindow::handleSearchMatched, Qt::QueuedConnection);
    connect(searchReplaceWorker.data(), &SearchReplaceWorker::searchFinished, q, &FindToolWindow::handleSearchFinished, Qt::QueuedConnection);
    connect(searchReplaceWorker.data(), &SearchReplaceWorker::replaceFinished, q, &FindToolWindow::handleReplaceFinished, Qt::QueuedConnection);

    searchReplaceWorker->moveToThread(&thread);
    thread.start();
}

QWidget *FindToolWindowPrivate::createSearchParamWidget()
{
    QWidget *widget = new QWidget(q);
    QHBoxLayout *mainLayout = new QHBoxLayout(widget);

    DLabel *scopeLabel = new DLabel(FindToolWindow::tr("Scope:"));
    scopeComboBox = new DComboBox(q);
    scopeComboBox->addItem(tr("All Projects"), AllProjects);
    scopeComboBox->addItem(tr("Current Project"), CurrentProject);
    scopeComboBox->addItem(tr("Current File"), CurrentFile);
    scopeComboBox->setFixedWidth(369);

    DLabel *searchLabel = new DLabel(FindToolWindow::tr("Search for:"));
    QHBoxLayout *hlayout = new QHBoxLayout();
    searchLineEdit = new DLineEdit(q);
    searchLineEdit->setFixedWidth(277);
    searchLineEdit->setPlaceholderText(FindToolWindow::tr("thread"));

    caseSensitiveCheckBtn = new DPushButton(q);
    caseSensitiveCheckBtn->setToolTip(FindToolWindow::tr("Case sensitive"));
    caseSensitiveCheckBtn->setIcon(QIcon::fromTheme("match_case"));
    caseSensitiveCheckBtn->setIconSize({ 16, 16 });
    caseSensitiveCheckBtn->setCheckable(true);

    wholeWordsCheckBtn = new DPushButton(q);
    wholeWordsCheckBtn->setToolTip(FindToolWindow::tr("Whole words only"));
    wholeWordsCheckBtn->setIcon(QIcon::fromTheme("find_matchComplete"));
    wholeWordsCheckBtn->setIconSize({ 16, 16 });
    wholeWordsCheckBtn->setCheckable(true);

    hlayout->addWidget(searchLineEdit);
    hlayout->addWidget(caseSensitiveCheckBtn);
    hlayout->addWidget(wholeWordsCheckBtn);

    DLabel *patternLabel = new DLabel(FindToolWindow::tr("File pattern:"), q);
    patternLineEdit = new DLineEdit(q);
    patternLineEdit->setPlaceholderText(FindToolWindow::tr("e.g.*.ts,src/**/include"));
    patternLineEdit->setFixedWidth(369);

    DLabel *expatternLabel = new DLabel(FindToolWindow::tr("Exclusion pattern:"), q);
    expatternLineEdit = new DLineEdit(q);
    expatternLineEdit->setPlaceholderText(FindToolWindow::tr("e.g.*.ts,src/**/include"));
    expatternLineEdit->setFixedWidth(369);

    searchBtn = new DPushButton(FindToolWindow::tr("Search", "button"), q);
    searchBtn->setMinimumWidth(120);
    replaceBtn = new DPushButton(FindToolWindow::tr("Search && Replace", "button"), q);
    replaceBtn->setMinimumWidth(120);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addStretch(1);
    btnLayout->addWidget(searchBtn);
    btnLayout->addWidget(replaceBtn);

    QFormLayout *formLayout = new QFormLayout(widget);
    formLayout->setContentsMargins(26, 10, 0, 0);
    formLayout->setSpacing(10);
    formLayout->addRow(scopeLabel, scopeComboBox);
    formLayout->addRow(searchLabel, hlayout);
    formLayout->addRow(patternLabel, patternLineEdit);
    formLayout->addRow(expatternLabel, expatternLineEdit);
    formLayout->addRow(btnLayout);
    formLayout->setAlignment(btnLayout, Qt::AlignLeft);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch(1);
    return widget;
}

QWidget *FindToolWindowPrivate::createSearchResultWidget()
{
    QWidget *widget = new QWidget(q);
    QVBoxLayout *vLayout = new QVBoxLayout(widget);

    searchResultWindow = new SearchResultWindow(q);
    vLayout->addWidget(searchResultWindow);

    return widget;
}

bool FindToolWindowPrivate::checkSelectedScopeValid()
{
    int scope = scopeComboBox->currentData().toInt();
    switch (scope) {
    case AllProjects: {
        auto projectSrv = dpfGetService(ProjectService);
        const auto &infoList = projectSrv->getAllProjectInfo();
        if (infoList.isEmpty()) {
            showMessage(FindToolWindow::tr("All projects path is empty, please import!"));
            return false;
        }
        break;
    }
    case CurrentProject: {
        auto projectSrv = dpfGetService(ProjectService);
        const auto &info = projectSrv->getActiveProjectInfo();
        if (info.isEmpty()) {
            showMessage(FindToolWindow::tr("Current project path is empty, please import!"));
            return false;
        }
        break;
    }
    case CurrentFile: {
        auto editSrv = dpfGetService(EditorService);
        auto curFile = editSrv->currentFile();
        if (curFile.isEmpty()) {
            showMessage(FindToolWindow::tr("No files are currently open, please open!"));
            return false;
        }
        break;
    }
    default: {
        showMessage(FindToolWindow::tr("Scope is not selected, please select!"));
        return false;
    }
    }

    return true;
}

bool FindToolWindowPrivate::setBaseParams(BaseParams *params, bool onlyOpened)
{
    if (!params)
        return false;

    params->keyword = searchLineEdit->text();
    if (params->keyword.isEmpty()) {
        searchLineEdit->showAlertMessage(tr("Search for text is empty, please input!"));
        return false;
    }

    auto projectSrv = dpfGetService(ProjectService);
    auto editSrv = dpfGetService(EditorService);
    int scope = scopeComboBox->currentData().toInt();
    switch (scope) {
    case AllProjects: {
        params->openedFileList = editSrv->openedFiles();
        if (onlyOpened)
            return true;

        const auto &infoList = projectSrv->getAllProjectInfo();
        for (const auto &info : infoList) {
            params->baseFileList.append(info.sourceFiles().toList());
        }
        return true;
    }
    case CurrentProject: {
        params->openedFileList = editSrv->openedFiles();
        if (onlyOpened)
            return true;

        const auto &info = projectSrv->getActiveProjectInfo();
        params->baseFileList.append(info.sourceFiles().toList());
        return true;
    }
    case CurrentFile: {
        auto curFile = editSrv->currentFile();
        params->openedFileList << curFile;
        return true;
    }
    default: {
        break;
    }
    }

    return false;
}

void FindToolWindowPrivate::showMessage(const QString &message)
{
    DDialog dlg(q);
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.setMessage(message);
    dlg.insertButton(0, FindToolWindow::tr("Ok"));

    dlg.exec();
}

FindToolWindow::FindToolWindow(QWidget *parent)
    : QWidget(parent),
      d(new FindToolWindowPrivate(this))
{
    d->initUI();
    d->initWorker();
    d->initConnect();
}

FindToolWindow::~FindToolWindow()
{
    delete d;
}

void FindToolWindow::search()
{
    searchText();
    d->searchResultWindow->setRepalceWidgtVisible(false);
}

bool FindToolWindow::getSearchParams(SearchParams *searchParams)
{
    if (!d->checkSelectedScopeValid())
        return false;

    if (!d->setBaseParams(&searchParams->baseParams))
        return false;

    searchParams->scope = static_cast<SearchScope>(d->scopeComboBox->currentData().toInt());
    searchParams->flags |= d->caseSensitiveCheckBtn->isChecked() ? SearchCaseSensitive : SearchNoFlag;
    searchParams->flags |= d->wholeWordsCheckBtn->isChecked() ? SearchWholeWord : SearchNoFlag;
    searchParams->includeList = d->patternLineEdit->text().trimmed().split(",", QString::SkipEmptyParts);
    searchParams->excludeList = d->expatternLineEdit->text().trimmed().split(",", QString::SkipEmptyParts);

    return true;
}

void FindToolWindow::searchText()
{
    SearchParams params;
    if (!getSearchParams(&params)) {
        return;
    }

    d->searchResultWindow->showMsg(true, tr("Searching, please wait..."));
    d->searchResultWindow->clear();
    metaObject()->invokeMethod(d->searchReplaceWorker.data(),
                               "addSearchTask",
                               Qt::QueuedConnection,
                               Q_ARG(SearchParams, params));
    d->stackedWidget->setCurrentIndex(1);
}

void FindToolWindow::replace()
{
    if (!d->checkSelectedScopeValid())
        return;

    searchText();
    d->searchResultWindow->setRepalceWidgtVisible(true);
}

void FindToolWindow::switchSearchParamWidget()
{
    d->searchReplaceWorker->stop();
    d->stackedWidget->setCurrentIndex(0);
}

void FindToolWindow::handleSearchMatched()
{
    const auto &results = d->searchReplaceWorker->getResults();
    if (results.isEmpty())
        return;

    d->searchResultWindow->appendResults(results);
}

void FindToolWindow::handleSearchFinished()
{
    d->searchResultWindow->searchFinished();
}

void FindToolWindow::handleReplace(const QString &text)
{
    if (!d->checkSelectedScopeValid())
        return;

    ReplaceParams params;
    if (!d->setBaseParams(&params.baseParams, true))
        return;

    params.replaceText = text;
    params.baseParams.baseFileList = d->searchResultWindow->resultFileList();
    params.flags |= d->caseSensitiveCheckBtn->isChecked() ? SearchCaseSensitive : SearchNoFlag;
    params.flags |= d->wholeWordsCheckBtn->isChecked() ? SearchWholeWord : SearchNoFlag;
    metaObject()->invokeMethod(d->searchReplaceWorker.data(),
                               "addReplaceTask",
                               Qt::QueuedConnection,
                               Q_ARG(ReplaceParams, params));
}

void FindToolWindow::handleReplaceFinished(int result)
{
    d->searchResultWindow->replaceFinished(result == 0);
}
