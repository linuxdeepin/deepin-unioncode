// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "findtoolwindow.h"
#include "searchresultwindow.h"
#include "transceiver/findreceiver.h"
#include "util/searcreplacehworker.h"

#include <DLineEdit>
#include <DLabel>
#include <DComboBox>
#include <DCheckBox>
#include <DPushButton>
#include <DStackedWidget>
#include <DMessageBox>
#include <DScrollArea>
#include <DSuggestButton>
#include <DFrame>
#include <DDialog>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QProcess>
#include <QFormLayout>
#include <QThread>

DWIDGET_USE_NAMESPACE
class FindToolWindowPrivate
{
    FindToolWindowPrivate() {}
    ~FindToolWindowPrivate();

    DStackedWidget *stackedWidget { nullptr };
    SearchResultWindow *searchResultWindow { nullptr };
    QSet<QString> allProjectsPathList { nullptr };
    QString currentProjectPath;
    QString currentFilePath;
    QMap<QString, QString> projectInfoMap;
    QSharedPointer<SearchReplaceWorker> searchReplaceWorker { nullptr };
    QThread thread;

    DComboBox *scopeComboBox { nullptr };
    DLineEdit *searchLineEdit { nullptr };
    DCheckBox *senseCheckBox { nullptr };
    DCheckBox *wholeWordsCheckBox { nullptr };
    DCheckBox *regularCheckBox { nullptr };
    DLineEdit *patternLineEdit { nullptr };
    DLineEdit *expatternLineEdit { nullptr };
    DSuggestButton *senseCheckBtnOn { nullptr };
    DPushButton *senseCheckBtnOff { nullptr };
    DSuggestButton *wholeWordsCheckBtnOn { nullptr };
    DPushButton *wholeWordsCheckBtnOff { nullptr };

    bool senseCheckBtnFlag = false;
    bool wholeWordsCheckBtnFlag = false;
    friend class FindToolWindow;
};

FindToolWindowPrivate::~FindToolWindowPrivate()
{
    searchReplaceWorker->stop();
    thread.quit();
    thread.wait();
}

FindToolWindow::FindToolWindow(QWidget *parent)
    : QWidget(parent),
      d(new FindToolWindowPrivate())
{
    setupUi();
    initWorker();
    connect(FindEventTransmit::instance(), QOverload<const QString &, const QString &>::of(&FindEventTransmit::sendProjectPath),
            [=](const QString &projectPath, const QString &language) {
                d->currentProjectPath = projectPath;
                d->projectInfoMap.insert(projectPath, language);
                d->allProjectsPathList.insert(projectPath);
            });

    connect(FindEventTransmit::instance(), QOverload<const QString &>::of(&FindEventTransmit::sendRemovedProject),
            [=](const QString &projectPath) {
                d->currentProjectPath = "";
                d->allProjectsPathList.remove(projectPath);
                d->projectInfoMap.remove(projectPath);
            });

    connect(FindEventTransmit::instance(), QOverload<const QString &, bool>::of(&FindEventTransmit::sendCurrentEditFile),
            [=](const QString &filePath, bool actived) {
                if (actived) {
                    d->currentFilePath = filePath;
                } else
                    d->currentFilePath = "";
            });
}

FindToolWindow::~FindToolWindow()
{
    delete d;
}

void FindToolWindow::setupUi()
{
    DFrame *mainPaneFrame = new DFrame(this);
    DStyle::setFrameRadius(mainPaneFrame, 0);
    d->stackedWidget = new DStackedWidget();
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);

    DScrollArea *scrollArea = new DScrollArea(mainPaneFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(d->stackedWidget);
    scrollArea->setLineWidth(0);

    mainPaneFrame->setLineWidth(0);
    hLayout->addWidget(mainPaneFrame);
    QHBoxLayout *scrollLayout = new QHBoxLayout();
    scrollLayout->addWidget(scrollArea);
    mainPaneFrame->setLayout(scrollLayout);

    QWidget *searchParamWidget = new QWidget();
    QWidget *searchResultWidget = new QWidget();

    addSearchParamWidget(searchParamWidget);
    addSearchResultWidget(searchResultWidget);

    d->stackedWidget->addWidget(searchParamWidget);
    d->stackedWidget->addWidget(searchResultWidget);
    d->stackedWidget->setCurrentIndex(0);

    setLayout(hLayout);
}

void FindToolWindow::initWorker()
{
    d->searchReplaceWorker.reset(new SearchReplaceWorker);
    connect(d->searchReplaceWorker.data(), &SearchReplaceWorker::matched, this, &FindToolWindow::handleSearchMatched, Qt::QueuedConnection);
    connect(d->searchReplaceWorker.data(), &SearchReplaceWorker::searchFinished, this, &FindToolWindow::handleSearchFinished, Qt::QueuedConnection);
    connect(d->searchReplaceWorker.data(), &SearchReplaceWorker::replaceFinished, this, &FindToolWindow::handleReplaceFinished, Qt::QueuedConnection);

    d->searchReplaceWorker->moveToThread(&d->thread);
    d->thread.start();
}

void FindToolWindow::addSearchParamWidget(QWidget *parentWidget)
{
    QFormLayout *formLayout = new QFormLayout();
    parentWidget->setLayout(formLayout);

    DLabel *scopeLabel = new DLabel(QLabel::tr("Scope:"));
    d->scopeComboBox = new DComboBox(parentWidget);
    d->scopeComboBox->addItem(tr("All Projects"));
    d->scopeComboBox->addItem(tr("Current Project"));
    d->scopeComboBox->addItem(tr("Current File"));
    d->scopeComboBox->setFixedWidth(369);

    DLabel *searchLabel = new DLabel(QLabel::tr("Search for:"));
    QHBoxLayout *hlayout = new QHBoxLayout();
    d->searchLineEdit = new DLineEdit(parentWidget);
    d->searchLineEdit->setFixedWidth(277);
    d->searchLineEdit->setPlaceholderText(tr("thread"));

    d->senseCheckBtnOn = new DSuggestButton(parentWidget);
    d->senseCheckBtnOn->setText("Aa");
    d->senseCheckBtnOn->setFixedSize(36, 36);
    d->senseCheckBtnOn->hide();

    d->senseCheckBtnOff = new DPushButton(parentWidget);
    d->senseCheckBtnOff->setText("aa");
    d->senseCheckBtnOff->setFixedSize(36, 36);

    d->wholeWordsCheckBtnOn = new DSuggestButton(parentWidget);
    d->wholeWordsCheckBtnOn->setIcon(QIcon::fromTheme("find_matchComplete"));
    d->wholeWordsCheckBtnOn->setFixedSize(36, 36);
    d->wholeWordsCheckBtnOn->hide();

    d->wholeWordsCheckBtnOff = new DPushButton(parentWidget);
    d->wholeWordsCheckBtnOff->setIcon(QIcon::fromTheme("find_matchComplete"));
    d->wholeWordsCheckBtnOff->setFixedSize(36, 36);

    hlayout->addWidget(d->searchLineEdit);
    hlayout->addWidget(d->senseCheckBtnOn);
    hlayout->addWidget(d->senseCheckBtnOff);
    hlayout->addWidget(d->wholeWordsCheckBtnOn);
    hlayout->addWidget(d->wholeWordsCheckBtnOff);

    DLabel *patternLabel = new DLabel(QLabel::tr("File pattern:"), parentWidget);
    d->patternLineEdit = new DLineEdit(parentWidget);
    d->patternLineEdit->setPlaceholderText(tr("e.g.*.ts,src/**/include"));
    d->patternLineEdit->setFixedWidth(369);

    DLabel *expatternLabel = new DLabel(QLabel::tr("Exclusion pattern:"), parentWidget);
    d->expatternLineEdit = new DLineEdit(parentWidget);
    d->expatternLineEdit->setPlaceholderText(tr("e.g.*.ts,src/**/include"));
    d->expatternLineEdit->setFixedWidth(369);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    DPushButton *searchBtn = new DPushButton(QPushButton::tr("Search"), parentWidget);
    searchBtn->setMinimumWidth(120);
    searchBtn->setMaximumWidth(120);
    searchBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    DPushButton *replaceBtn = new DPushButton(QPushButton::tr("Search && Replace"), parentWidget);
    replaceBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    replaceBtn->setMinimumWidth(120);
    replaceBtn->setMaximumWidth(120);

    btnLayout->addWidget(searchBtn);
    btnLayout->addWidget(replaceBtn);
    btnLayout->setContentsMargins(213, 0, 0, 0);

    connect(searchBtn, &QAbstractButton::clicked, this, &FindToolWindow::search);
    connect(replaceBtn, &QAbstractButton::clicked, this, &FindToolWindow::replace);

    connect(d->senseCheckBtnOn, &QPushButton::clicked, this, &FindToolWindow::onSenseCheckBtnClicked);
    connect(d->wholeWordsCheckBtnOn, &QPushButton::clicked, this, &FindToolWindow::onwholeWordsCheckBtnClicked);
    connect(d->senseCheckBtnOff, &QPushButton::clicked, this, &FindToolWindow::onSenseCheckBtnClicked);
    connect(d->wholeWordsCheckBtnOff, &QPushButton::clicked, this, &FindToolWindow::onwholeWordsCheckBtnClicked);

    formLayout->setContentsMargins(26, 10, 0, 0);
    formLayout->setSpacing(10);
    formLayout->addRow(scopeLabel, d->scopeComboBox);
    formLayout->addRow(searchLabel, hlayout);
    formLayout->addRow(patternLabel, d->patternLineEdit);
    formLayout->addRow(expatternLabel, d->expatternLineEdit);
    formLayout->addRow(btnLayout);
    formLayout->setAlignment(btnLayout, Qt::AlignLeft);
}

void FindToolWindow::onSenseCheckBtnClicked()
{
    d->senseCheckBtnFlag = !d->senseCheckBtnFlag;
    if (d->senseCheckBtnFlag) {
        d->senseCheckBtnOff->hide();
        d->senseCheckBtnOn->show();
    } else {
        d->senseCheckBtnOn->hide();
        d->senseCheckBtnOff->show();
    }
}

void FindToolWindow::onwholeWordsCheckBtnClicked()
{
    d->wholeWordsCheckBtnFlag = !d->wholeWordsCheckBtnFlag;
    if (d->wholeWordsCheckBtnFlag) {
        d->wholeWordsCheckBtnOff->hide();
        d->wholeWordsCheckBtnOn->show();
    } else {
        d->wholeWordsCheckBtnOn->hide();
        d->wholeWordsCheckBtnOff->show();
    }
}

void FindToolWindow::addSearchResultWidget(QWidget *parentWidget)
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    parentWidget->setLayout(vLayout);

    d->searchResultWindow = new SearchResultWindow();
    connect(d->searchResultWindow, &SearchResultWindow::reqBack, this, &FindToolWindow::switchSearchParamWidget);
    connect(d->searchResultWindow, &SearchResultWindow::reqReplace, this, &FindToolWindow::handleReplace);
    vLayout->addWidget(d->searchResultWindow);
}

void FindToolWindow::search()
{
    searchText();
    d->searchResultWindow->setRepalceWidgtVisible(false);
}

void FindToolWindow::createMessageDialog(const QString &message)
{
    DDialog *messageDialog = new DDialog(this);
    messageDialog->setIcon(QIcon::fromTheme("dialog-warning"));
    messageDialog->setMessage(message);
    messageDialog->insertButton(0, tr("Ok"));
    messageDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(messageDialog, &DDialog::buttonClicked, [=](int index) {
        if (index == 0) {
            messageDialog->reject();
        }
    });

    messageDialog->exec();
}

bool FindToolWindow::checkSelectedScopeValid()
{
    int index = d->scopeComboBox->currentIndex();
    switch (index) {
    case 0: {
        if (d->allProjectsPathList.isEmpty()) {
            createMessageDialog(tr("All projects path is empty, please import!"));
            return false;
        }
        break;
    }
    case 1: {
        if (d->currentProjectPath.isEmpty()) {
            createMessageDialog(tr("Current project path is empty, please import!"));
            return false;
        }
        break;
    }
    case 2: {
        if (d->currentFilePath.isEmpty()) {
            createMessageDialog(tr("Current project path is empty, please import!"));
            return false;
        }
        break;
    }
    default: {
        createMessageDialog(tr("Scope is not selected, please select!"));
        return false;
    }
    }

    return true;
}

bool FindToolWindow::getSearchParams(SearchParams *searchParams)
{
    if (!checkSelectedScopeValid())
        return false;
    QString text = d->searchLineEdit->text();
    if (text.isEmpty()) {
        d->searchLineEdit->showAlertMessage(tr("Search for text is empty, please input!"));
        return false;
    }

    QStringList searchPathList;
    int index = d->scopeComboBox->currentIndex();
    switch (index) {
    case 0:
        searchPathList = d->allProjectsPathList.values();
        break;
    case 1:
        searchPathList = QStringList { d->currentProjectPath };
        break;
    case 2:
        searchPathList = QStringList { d->currentFilePath };
        break;
    default:
        break;
    }

    searchParams->filePathList = searchPathList;
    searchParams->searchText = text;
    searchParams->sensitiveFlag = d->senseCheckBtnFlag;
    searchParams->wholeWordsFlag = d->wholeWordsCheckBtnFlag;
    searchParams->patternsList = d->patternLineEdit->text().trimmed().split(",", QString::SkipEmptyParts);
    searchParams->exPatternsList = d->expatternLineEdit->text().trimmed().split(",", QString::SkipEmptyParts);
    searchParams->exPatternsList << "*.so" << "*.o";
    searchParams->projectInfoMap = d->projectInfoMap;

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
    if (!checkSelectedScopeValid())
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

    d->searchResultWindow->appendResults(results, d->projectInfoMap);
}

void FindToolWindow::handleSearchFinished()
{
    d->searchResultWindow->searchFinished();
}

void FindToolWindow::handleReplace(const QString &text)
{
    ReplaceParams params;
    params.replaceText = text;
    params.searchText = d->searchLineEdit->text();
    int index = d->scopeComboBox->currentIndex();
    switch (index) {
    case 0:
        params.filePathList = d->allProjectsPathList.values();
        break;
    case 1:
        params.filePathList = QStringList { d->currentProjectPath };
        break;
    case 2:
        params.filePathList = QStringList { d->currentFilePath };
        break;
    default:
        break;
    }
    metaObject()->invokeMethod(d->searchReplaceWorker.data(),
                               "addReplaceTask",
                               Qt::QueuedConnection,
                               Q_ARG(ReplaceParams, params));
}

void FindToolWindow::handleReplaceFinished(int result)
{
    d->searchResultWindow->replaceFinished(result == 0);
}
