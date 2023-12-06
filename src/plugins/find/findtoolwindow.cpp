// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "findtoolwindow.h"
#include "searchresultwindow.h"
#include "transceiver/findreceiver.h"

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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QProcess>
#include <QFormLayout>

DWIDGET_USE_NAMESPACE
class FindToolWindowPrivate
{
    FindToolWindowPrivate(){}
    DStackedWidget *stackedWidget{nullptr};
    SearchResultWindow *searchResultWindow{nullptr};
    QSet<QString> allProjectsPathList{nullptr};
    QString currentProjectPath;
    QString currentFilePath;
    QMap<QString, QString> projectInfoMap;

    DComboBox *scopeComboBox{nullptr};
    DLineEdit *searchLineEdit{nullptr};
    DCheckBox *senseCheckBox{nullptr};
    DCheckBox *wholeWordsCheckBox{nullptr};
    DCheckBox *regularCheckBox{nullptr};
    DLineEdit *patternLineEdit{nullptr};
    DLineEdit *expatternLineEdit{nullptr};
    DSuggestButton *senseCheckBtn{nullptr};
    DPushButton *wholeWordsCheckBtn{nullptr};
    friend class FindToolWindow;
};

FindToolWindow::FindToolWindow(QWidget *parent)
    : QWidget(parent)
    , d(new FindToolWindowPrivate())
{
    setupUi();
    connect(FindEventTransmit::instance(), QOverload<const QString &, const QString &>::of(&FindEventTransmit::sendProjectPath),
            [=](const QString &projectPath, const QString &language){
        d->currentProjectPath = projectPath;
        d->projectInfoMap.insert(projectPath, language);
        d->allProjectsPathList.insert(projectPath);
    });

    connect(FindEventTransmit::instance(), QOverload<const QString &>::of(&FindEventTransmit::sendRemovedProject),
            [=](const QString &projectPath){
        d->currentProjectPath = "";
        d->allProjectsPathList.remove(projectPath);
        d->projectInfoMap.remove(projectPath);
    });

    connect(FindEventTransmit::instance(), QOverload<const QString &, bool>::of(&FindEventTransmit::sendCurrentEditFile),
            [=](const QString &filePath, bool actived){
        if (actived) {
            d->currentFilePath = filePath;
        }
        else
            d->currentFilePath = "";
    });
}

void FindToolWindow::setupUi()
{
    d->stackedWidget = new DStackedWidget();
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);

    DScrollArea *scrollArea = new DScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(d->stackedWidget);
    vLayout->addWidget(scrollArea);

    QWidget *searchParamWidget = new QWidget();
    QWidget *searchResultWidget = new QWidget();

    addSearchParamWidget(searchParamWidget);
    addSearchResultWidget(searchResultWidget);

    d->stackedWidget->addWidget(searchParamWidget);
    d->stackedWidget->addWidget(searchResultWidget);
    d->stackedWidget->setCurrentIndex(0);

    setLayout(vLayout);
}

void FindToolWindow::addSearchParamWidget(QWidget *parentWidget)
{
    parentWidget->setFixedSize(458, 220);

    QFormLayout *formLayout = new QFormLayout();
    parentWidget->setLayout(formLayout);

    DLabel *scopeLabel = new DLabel(QLabel::tr("Scope:"));
    d->scopeComboBox = new DComboBox();
    d->scopeComboBox->addItem(tr("All Projects"));
    d->scopeComboBox->addItem(tr("Current Project"));
    d->scopeComboBox->addItem(tr("Current File"));

    DLabel *searchLabel = new DLabel(QLabel::tr("Search for:"));
    QHBoxLayout *hlayout = new QHBoxLayout();
    d->searchLineEdit = new DLineEdit();
    d->searchLineEdit->setPlaceholderText(tr("thread"));

    d->senseCheckBtn = new DSuggestButton();
    d->senseCheckBtn->setText("Aa");
    d->senseCheckBtn->setFixedSize(36,36);
    d->wholeWordsCheckBtn = new DPushButton();
    d->wholeWordsCheckBtn->setIcon(QIcon::fromTheme("find_matchComplete"));
    d->wholeWordsCheckBtn->setFixedSize(36,36);
    hlayout->addWidget(d->searchLineEdit);
    hlayout->addWidget(d->senseCheckBtn);
    hlayout->addWidget(d->wholeWordsCheckBtn);

    DLabel *patternLabel = new DLabel(QLabel::tr("File pattern:"));
    d->patternLineEdit = new DLineEdit();
    d->patternLineEdit->setPlaceholderText(tr("e.g.*.ts,src/**/include"));

    DLabel *expatternLabel = new DLabel(QLabel::tr("Exclusion pattern:"));
    d->expatternLineEdit = new DLineEdit();
    d->expatternLineEdit->setPlaceholderText(tr("e.g.*.ts,src/**/include"));

    QHBoxLayout *btnLayout = new QHBoxLayout();
    DPushButton *searchBtn = new DPushButton(QPushButton::tr("Search"));
    searchBtn->setFixedSize(130, 36);
    DPushButton *replaceBtn = new DPushButton(QPushButton::tr("Search && Replace"));
    replaceBtn->setFixedSize(130,36);
    btnLayout->addWidget(searchBtn);
    btnLayout->addWidget(replaceBtn);

    connect(searchBtn, &QAbstractButton::clicked, this, &FindToolWindow::search);
    connect(replaceBtn, &QAbstractButton::clicked, this, &FindToolWindow::replace);

    formLayout->setContentsMargins(26, 10, 0, 0);
    formLayout->setSpacing(10);
    formLayout->addRow(scopeLabel, d->scopeComboBox);
    formLayout->addRow(searchLabel, hlayout);
    formLayout->addRow(patternLabel, d->patternLineEdit);
    formLayout->addRow(expatternLabel, d->expatternLineEdit);
    formLayout->addRow(btnLayout);
    formLayout->setAlignment(btnLayout, Qt::AlignRight);

}

void FindToolWindow::addSearchResultWidget(QWidget *parentWidget)
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    parentWidget->setLayout(vLayout);

    d->searchResultWindow = new SearchResultWindow();
    connect(d->searchResultWindow, &SearchResultWindow::back, this, &FindToolWindow::switchSearchParamWidget);
    vLayout->addWidget(d->searchResultWindow);
}

void FindToolWindow::search()
{
    searchText();
    d->searchResultWindow->setRepalceWidgtVisible(false);
}

bool FindToolWindow::checkSelectedScopeValid()
{
    int index = d->scopeComboBox->currentIndex();
    switch (index) {
    case 0:
    {
        if (d->allProjectsPathList.isEmpty()) {
            DMessageBox::warning(this, tr("Error"), tr("All projects path is empty, please import!"));
            return false;
        }
        break;
    }
    case 1:
    {
        if (d->currentProjectPath.isEmpty()) {
            DMessageBox::warning(this, tr("Error"), tr("Current project path is empty, please import!"));
            return false;
        }
        break;
    }
    case 2:
    {
        if (d->currentFilePath.isEmpty()) {
            DMessageBox::warning(this, tr("Error"), tr("Current file path is empty, please import!"));
            return false;
        }
        break;
    }
    default:
    {
        DMessageBox::warning(this, tr("Error"), tr("Scope is not selected, please select!"));
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
        DMessageBox::warning(this, tr("Error"), tr("Search for text is empty, please input!"));
        return false;
    }

    QStringList searchPathList;
    int index = d->scopeComboBox->currentIndex();
    switch (index) {
    case 0:
        searchPathList = d->allProjectsPathList.values();
        break;
    case 1:
        searchPathList = QStringList{d->currentProjectPath};
        break;
    case 2:
        searchPathList = QStringList{d->currentFilePath};
        break;
    default:
        break;
    }

    searchParams->filePathList = searchPathList;
    searchParams->searchText = text;
    searchParams->sensitiveFlag = d->senseCheckBtn->isChecked();
    searchParams->wholeWordsFlag  = d->wholeWordsCheckBtn->isChecked();
    searchParams->patternsList = d->patternLineEdit->text().trimmed().split(",", QString::SkipEmptyParts);
    searchParams->exPatternsList = d->expatternLineEdit->text().trimmed().split(",", QString::SkipEmptyParts);
    searchParams->projectInfoMap = d->projectInfoMap;

    return true;
}

void FindToolWindow::searchText()
{
    SearchParams params;
    if (!getSearchParams(&params)) {
        return;
    }

    d->searchResultWindow->search(&params);
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
    d->stackedWidget->setCurrentIndex(0);
}
