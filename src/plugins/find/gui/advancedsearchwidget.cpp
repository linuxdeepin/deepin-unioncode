// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "advancedsearchwidget.h"
#include "searchresultwidget.h"
#include "maincontroller/maincontroller.h"

#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/editor/editorservice.h"

#include <DComboBox>
#include <DLabel>
#include <DDialog>
#include <DLineEdit>
#include <DPushButton>
#include <DToolButton>
#include <DSpinner>

#include <QWidgetAction>
#include <QVBoxLayout>

using namespace dpfservice;
DWIDGET_USE_NAMESPACE

class AdvancedSearchWidgetPrivate : public QObject
{
public:
    explicit AdvancedSearchWidgetPrivate(AdvancedSearchWidget *qq);

    void initUI();
    void initConnection();
    void initOperator();

    DToolButton *registerOperator(const QIcon &icon, const QString &description, std::function<void()> handler);
    QWidget *createSearchParamWidget();
    QWidget *createOptionWidget();
    DToolButton *createOptionButton(const QIcon &icon, const QString &description);

    SearchParams searchParams();
    ReplaceParams replaceParams(const QMap<QString, FindItemList> &resultMap);
    bool checkSearchParamsVaild(const SearchParams &params);
    int showMessage(const QString &msg);
    void toggleSearchState(bool searching);
    void updateReplaceInfo();

    void refresh();
    void search();
    void replace(const QMap<QString, FindItemList> &resultMap);
    void stop();
    void clear();

    void handleSearchMatched();
    void handleSearchFinished();
    void handleReplaceAll();
    void handleReplaceFinished();

public:
    AdvancedSearchWidget *q;

    // header
    DToolButton *refreshBtn { nullptr };
    DToolButton *stopSearchBtn { nullptr };
    DToolButton *expandBtn { nullptr };
    DToolButton *collapseBtn { nullptr };
    DToolButton *clearBtn { nullptr };
    DSpinner *searchSpinner { nullptr };

    // top
    DLineEdit *searchEdit { nullptr };
    DLineEdit *replaceEdit { nullptr };
    DPushButton *replaceAllBtn { nullptr };
    DComboBox *searchScopeCB { nullptr };
    DLineEdit *includeEdit { nullptr };
    DLineEdit *excludeEdit { nullptr };
    QWidget *optionWidget { nullptr };

    DToolButton *caseBtn { nullptr };
    DToolButton *wholeWordBtn { nullptr };
    DToolButton *regexBtn { nullptr };
    DToolButton *optionBtn { nullptr };

    // result
    SearchResultWidget *resultWidget { nullptr };

    // util
    WindowService *winSrv { nullptr };
    MainController *controller { nullptr };
    QTimer startTimer;
};

AdvancedSearchWidgetPrivate::AdvancedSearchWidgetPrivate(AdvancedSearchWidget *qq)
    : q(qq),
      controller(new MainController(q))
{
    startTimer.setSingleShot(true);
    startTimer.setInterval(500);
}

void AdvancedSearchWidgetPrivate::initUI()
{
    q->setAutoFillBackground(true);
    q->setBackgroundRole(QPalette::Base);

    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(10, 5, 10, 0);

    auto widget = createSearchParamWidget();
    resultWidget = new SearchResultWidget(q);

    mainLayout->addWidget(widget);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(resultWidget, 1);
}

void AdvancedSearchWidgetPrivate::initConnection()
{
    connect(&startTimer, &QTimer::timeout, this, &AdvancedSearchWidgetPrivate::search);
    connect(searchEdit, &DLineEdit::textChanged, this, &AdvancedSearchWidgetPrivate::refresh);
    connect(includeEdit, &DLineEdit::textChanged, this, &AdvancedSearchWidgetPrivate::refresh);
    connect(excludeEdit, &DLineEdit::textChanged, this, &AdvancedSearchWidgetPrivate::refresh);
    connect(searchScopeCB, &DComboBox::currentTextChanged, this, &AdvancedSearchWidgetPrivate::refresh);
    connect(refreshBtn, &DToolButton::clicked, this, &AdvancedSearchWidgetPrivate::refresh);
    connect(caseBtn, &DToolButton::clicked, this, &AdvancedSearchWidgetPrivate::refresh);
    connect(wholeWordBtn, &DToolButton::clicked, this, &AdvancedSearchWidgetPrivate::refresh);
    connect(optionBtn, &DToolButton::clicked, this, [this] { optionWidget->setVisible(optionBtn->isChecked()); });
    connect(replaceAllBtn, &DToolButton::clicked, this, &AdvancedSearchWidgetPrivate::handleReplaceAll);
    connect(replaceEdit, &DLineEdit::textEdited, this, &AdvancedSearchWidgetPrivate::updateReplaceInfo);
    connect(regexBtn, &DToolButton::clicked, this, [this] {
        refresh();
        updateReplaceInfo();
    });

    connect(stopSearchBtn, &DToolButton::clicked, this, &AdvancedSearchWidgetPrivate::stop);
    connect(controller, &MainController::matched, this, &AdvancedSearchWidgetPrivate::handleSearchMatched);
    connect(controller, &MainController::searchFinished, this, &AdvancedSearchWidgetPrivate::handleSearchFinished);
    connect(controller, &MainController::replaceFinished, this, &AdvancedSearchWidgetPrivate::handleReplaceFinished);
    connect(resultWidget, &SearchResultWidget::requestReplace, this, &AdvancedSearchWidgetPrivate::replace);
    connect(resultWidget, &SearchResultWidget::resultCountChanged, this,
            [this] {
                // Not updated in search
                if (!searchSpinner->isPlaying())
                    replaceAllBtn->setEnabled(!resultWidget->isEmpty());
            });
}

void AdvancedSearchWidgetPrivate::initOperator()
{
    collapseBtn = registerOperator(QIcon::fromTheme("collapse_all"),
                                   AdvancedSearchWidget::tr("Collapse All"),
                                   std::bind(&SearchResultWidget::collapseAll, resultWidget));
    expandBtn = registerOperator(QIcon::fromTheme("expand_all"),
                                 AdvancedSearchWidget::tr("Expand All"),
                                 std::bind(&SearchResultWidget::expandAll, resultWidget));
    clearBtn = registerOperator(QIcon::fromTheme("delete"),
                                AdvancedSearchWidget::tr("Clear Search Results"),
                                std::bind(&AdvancedSearchWidgetPrivate::clear, this));
    refreshBtn = registerOperator(QIcon::fromTheme("refresh"),
                                  AdvancedSearchWidget::tr("Refresh"),
                                  std::bind(&AdvancedSearchWidgetPrivate::refresh, this));
    stopSearchBtn = registerOperator(QIcon::fromTheme("stop_search"),
                                     AdvancedSearchWidget::tr("Stop Search"),
                                     std::bind(&AdvancedSearchWidgetPrivate::stop, this));
    stopSearchBtn->hide();

    searchSpinner = new DSpinner(q);
    winSrv->registerWidgetToDockHeader(MWNA_ADVANCEDSEARCH, searchSpinner);
    searchSpinner->hide();
}

DToolButton *AdvancedSearchWidgetPrivate::registerOperator(const QIcon &icon, const QString &description, std::function<void()> handler)
{
    if (!winSrv)
        winSrv = dpfGetService(WindowService);

    DToolButton *btn = new DToolButton(q);
    btn->setIcon(icon);
    btn->setIconSize({ 16, 16 });
    btn->setToolTip(description);
    connect(btn, &DToolButton::clicked, this, handler);
    
    winSrv->registerWidgetToDockHeader(MWNA_ADVANCEDSEARCH, btn);
    return btn;
}

QWidget *AdvancedSearchWidgetPrivate::createSearchParamWidget()
{
    QWidget *widget = new QWidget(q);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    searchEdit = new DLineEdit(q);
    searchEdit->setPlaceholderText(AdvancedSearchWidget::tr("Search"));
    caseBtn = createOptionButton(QIcon::fromTheme("match_case"), AdvancedSearchWidget::tr("Match Case"));
    wholeWordBtn = createOptionButton(QIcon::fromTheme("whole_word"), AdvancedSearchWidget::tr("Match Whole Word"));
    regexBtn = createOptionButton(QIcon::fromTheme("regex"), AdvancedSearchWidget::tr("Use Regular Expression"));

    replaceEdit = new DLineEdit(q);
    replaceEdit->setPlaceholderText(AdvancedSearchWidget::tr("Replace"));
    replaceAllBtn = new DPushButton(AdvancedSearchWidget::tr("Replace All"), q);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->addWidget(searchEdit, 0, 0);
    gridLayout->addWidget(caseBtn, 0, 1);
    gridLayout->addWidget(wholeWordBtn, 0, 2);
    gridLayout->addWidget(regexBtn, 0, 3);
    gridLayout->addWidget(replaceEdit, 1, 0);
    gridLayout->addWidget(replaceAllBtn, 1, 1, 1, 3);

    searchScopeCB = new DComboBox(q);
    searchScopeCB->addItem(AdvancedSearchWidget::tr("All Projects"), AllProjects);
    searchScopeCB->addItem(AdvancedSearchWidget::tr("Current Project"), CurrentProject);
    searchScopeCB->addItem(AdvancedSearchWidget::tr("Current File"), CurrentFile);
    optionBtn = createOptionButton(QIcon::fromTheme("option"), AdvancedSearchWidget::tr("Toggle Search Details"));

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(searchScopeCB);
    hLayout->addWidget(optionBtn);

    optionWidget = createOptionWidget();
    optionWidget->setVisible(false);

    layout->addLayout(gridLayout);
    layout->addWidget(new DLabel(AdvancedSearchWidget::tr("Scope:")));
    layout->addLayout(hLayout);
    layout->addWidget(optionWidget);

    return widget;
}

QWidget *AdvancedSearchWidgetPrivate::createOptionWidget()
{
    QWidget *widget = new QWidget(q);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    includeEdit = new DLineEdit(q);
    includeEdit->setPlaceholderText(AdvancedSearchWidget::tr("e.g.*.ts,src/**/include"));

    excludeEdit = new DLineEdit(q);
    excludeEdit->setPlaceholderText(AdvancedSearchWidget::tr("e.g.*.ts,src/**/include"));

    layout->addWidget(new DLabel(AdvancedSearchWidget::tr("Files To Include:")));
    layout->addWidget(includeEdit);
    layout->addWidget(new DLabel(AdvancedSearchWidget::tr("Files To Exclude:")));
    layout->addWidget(excludeEdit);

    return widget;
}

DToolButton *AdvancedSearchWidgetPrivate::createOptionButton(const QIcon &icon, const QString &description)
{
    DToolButton *btn = new DToolButton(q);
    btn->setCheckable(true);
    btn->setIcon(icon);
    btn->setToolTip(description);

    return btn;
}

SearchParams AdvancedSearchWidgetPrivate::searchParams()
{
    SearchParams params;
    params.keyword = searchEdit->text();

    auto projectSrv = dpfGetService(ProjectService);
    auto editSrv = dpfGetService(EditorService);
    int scope = searchScopeCB->currentData().toInt();
    switch (scope) {
    case AllProjects: {
        params.editFileList = editSrv->openedFiles();

        const auto &infoList = projectSrv->getAllProjectInfo();
        for (const auto &info : infoList) {
            params.projectFileList.append(info.sourceFiles().toList());
        }
    } break;
    case CurrentProject: {
        params.editFileList = editSrv->openedFiles();

        const auto &info = projectSrv->getActiveProjectInfo();
        params.projectFileList.append(info.sourceFiles().toList());
    } break;
    case CurrentFile: {
        auto curFile = editSrv->currentFile();
        if (!curFile.isEmpty())
            params.editFileList << curFile;
    } break;
    default:
        break;
    }

    params.scope = static_cast<SearchScope>(scope);
    params.flags |= caseBtn->isChecked() ? SearchCaseSensitive : SearchNoFlag;
    params.flags |= wholeWordBtn->isChecked() ? SearchWholeWord : SearchNoFlag;
    params.flags |= regexBtn->isChecked() ? SearchRegularExpression : SearchNoFlag;
    params.includeList = includeEdit->text().trimmed().split(",", QString::SkipEmptyParts);
    params.excludeList = excludeEdit->text().trimmed().split(",", QString::SkipEmptyParts);

    return params;
}

ReplaceParams AdvancedSearchWidgetPrivate::replaceParams(const QMap<QString, FindItemList> &resultMap)
{
    auto editSrv = dpfGetService(EditorService);

    ReplaceParams params;
    params.editFileList = editSrv->openedFiles();
    params.resultMap = resultMap;
    params.replaceText = replaceEdit->text();
    params.flags |= caseBtn->isChecked() ? SearchCaseSensitive : SearchNoFlag;
    params.flags |= wholeWordBtn->isChecked() ? SearchWholeWord : SearchNoFlag;
    params.flags |= regexBtn->isChecked() ? SearchRegularExpression : SearchNoFlag;

    return params;
}

bool AdvancedSearchWidgetPrivate::checkSearchParamsVaild(const SearchParams &params)
{
    if (params.keyword.isEmpty())
        return false;

    switch (params.scope) {
    case AllProjects:
        if (params.projectFileList.isEmpty()) {
            resultWidget->showMessage(AdvancedSearchWidget::tr("All projects path is empty, please import!"), Warning);
            return false;
        }
        break;
    case CurrentProject:
        if (params.projectFileList.isEmpty()) {
            resultWidget->showMessage(AdvancedSearchWidget::tr("Current projects path is empty, please import!"), Warning);
            return false;
        }
        break;
    case CurrentFile:
        if (params.editFileList.isEmpty()) {
            resultWidget->showMessage(AdvancedSearchWidget::tr("No files are currently open, please open!"), Warning);
            return false;
        }
        break;
    default:
        break;
    }

    return true;
}

int AdvancedSearchWidgetPrivate::showMessage(const QString &msg)
{
    DDialog dlg;
    dlg.setMessage(msg);
    dlg.setWindowTitle(AdvancedSearchWidget::tr("Advance Search"));
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.setWordWrapMessage(true);
    dlg.addButton(AdvancedSearchWidget::tr("Cancel", "button"));
    dlg.addButton(AdvancedSearchWidget::tr("Continue", "button"), true, DDialog::ButtonWarning);
    return dlg.exec();
}

void AdvancedSearchWidgetPrivate::toggleSearchState(bool searching)
{
    searchSpinner->setVisible(searching);
    refreshBtn->setVisible(!searching);
    stopSearchBtn->setVisible(searching);
    replaceAllBtn->setEnabled(!searching && !resultWidget->isEmpty());
    if (searching) {
        searchSpinner->start();
    } else {
        searchSpinner->stop();
    }
}

void AdvancedSearchWidgetPrivate::updateReplaceInfo()
{
    resultWidget->setReplaceText(replaceEdit->text(), regexBtn->isChecked());
}

void AdvancedSearchWidgetPrivate::refresh()
{
    stop();
    resultWidget->clear();
    startTimer.start();
}

void AdvancedSearchWidgetPrivate::search()
{
    const auto &params = searchParams();
    if (!checkSearchParamsVaild(params))
        return;

    toggleSearchState(true);
    controller->search(params);
}

void AdvancedSearchWidgetPrivate::replace(const QMap<QString, FindItemList> &resultMap)
{
    if (searchSpinner->isPlaying())
        return;

    searchSpinner->setVisible(true);
    searchSpinner->start();
    const auto &params = replaceParams(resultMap);
    controller->replace(params);
}

void AdvancedSearchWidgetPrivate::stop()
{
    toggleSearchState(false);
    controller->stop();
}

void AdvancedSearchWidgetPrivate::clear()
{
    searchEdit->clear();
    replaceEdit->clear();
    resultWidget->clear();
}

void AdvancedSearchWidgetPrivate::handleSearchMatched()
{
    const auto &result = controller->takeAll();
    resultWidget->appendResults(result);
}

void AdvancedSearchWidgetPrivate::handleSearchFinished()
{
    toggleSearchState(false);
    if (resultWidget->isEmpty())
        resultWidget->showMessage(AdvancedSearchWidget::tr("No results found."));
}

void AdvancedSearchWidgetPrivate::handleReplaceAll()
{
    const auto &resultMap = resultWidget->allResult();
    int totalResults = 0;
    for (const auto &result : resultMap)
        totalResults += result.count();

    QString replaceText = replaceEdit->text();
    const int totalFiles = resultMap.count();
    QString msg;
    if (totalFiles == 1 && totalResults == 1) {
        msg = replaceText.isEmpty()
                ? AdvancedSearchWidget::tr("Replace 1 occurence across 1 file?")
                : AdvancedSearchWidget::tr("Replace 1 occurence across 1 file with %1?").arg(replaceText);
    } else if (totalFiles == 1) {
        msg = replaceText.isEmpty()
                ? AdvancedSearchWidget::tr("Replace %1 occurences across 1 file?").arg(totalResults)
                : AdvancedSearchWidget::tr("Replace %1 occurences across 1 file with %2?").arg(totalResults).arg(replaceText);
    } else {
        msg = replaceText.isEmpty()
                ? AdvancedSearchWidget::tr("Replace %1 occurences across %2 files?").arg(totalResults).arg(totalFiles)
                : AdvancedSearchWidget::tr("Replace %1 occurences across %2 files with %3?").arg(totalResults).arg(totalFiles).arg(replaceText);
    }
    // 0: cancel 1:contionue
    int code = showMessage(msg);
    if (1 == code)
        replace(resultMap);
}

void AdvancedSearchWidgetPrivate::handleReplaceFinished()
{
    searchSpinner->setVisible(false);
    searchSpinner->stop();
}

AdvancedSearchWidget::AdvancedSearchWidget(QWidget *parent)
    : QWidget(parent),
    d(new AdvancedSearchWidgetPrivate(this))
{
    d->initUI();
    d->initConnection();
}

AdvancedSearchWidget::~AdvancedSearchWidget()
{
    delete d;
}

void AdvancedSearchWidget::initOperator()
{
    d->initOperator();
}

QString AdvancedSearchWidget::searchText() const
{
    return d->searchEdit->text();
}

void AdvancedSearchWidget::setSearchText(const QString &text)
{
    d->searchEdit->setText(text);
}

void AdvancedSearchWidget::showEvent(QShowEvent *e)
{
    d->searchSpinner->setFixedSize(16, 16);
    d->searchEdit->setFocus();
    d->searchEdit->lineEdit()->selectAll();
    QWidget::showEvent(e);
}
