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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QProcess>

#define LABEL_WIDTH (120)
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
    QHBoxLayout *hLayout = new QHBoxLayout();
    parentWidget->setLayout(hLayout);

    QWidget *widget = new QWidget();
    widget->setFixedSize(725, 300);
    hLayout->addWidget(widget, 0, Qt::AlignLeft);

    hLayout->addWidget(widget);

    QVBoxLayout *vLayout = new QVBoxLayout();
    widget->setLayout(vLayout);

    QHBoxLayout *scopeLayout = new QHBoxLayout();
    DLabel *scopeLabel = new DLabel(QLabel::tr("Scope:"));
    scopeLabel->setFixedWidth(LABEL_WIDTH);
    d->scopeComboBox = new DComboBox();
    d->scopeComboBox->addItem(tr("All Projects"));
    d->scopeComboBox->addItem(tr("Current Project"));
    d->scopeComboBox->addItem(tr("Current File"));
    d->scopeComboBox->setFixedWidth(369);
    scopeLabel->setContentsMargins(72,0,0,0);
    scopeLayout->addWidget(scopeLabel);
    scopeLayout->addWidget(d->scopeComboBox);
    scopeLayout->setAlignment(Qt::AlignLeft );

    QHBoxLayout *searchLayout = new QHBoxLayout();
    DLabel *searchLabel = new DLabel(QLabel::tr("Search for:"));
    searchLabel->setFixedWidth(LABEL_WIDTH);
    searchLabel->setContentsMargins(45,0,0,0);
    d->searchLineEdit = new DLineEdit();
    d->searchLineEdit->setFixedWidth(287);
    d->searchLineEdit->setPlaceholderText(tr("thread"));
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(d->searchLineEdit);

    d->senseCheckBtn = new DSuggestButton(this);
    d->senseCheckBtn->setText("Aa");
    d->senseCheckBtn->setFixedSize(36,36);
    d->wholeWordsCheckBtn = new DPushButton(this);
    d->wholeWordsCheckBtn->setIcon(QIcon::fromTheme("find_matchComplete"));
    d->wholeWordsCheckBtn->setFixedSize(36,36);

    searchLayout->addWidget(d->senseCheckBtn);
    searchLayout->addWidget(d->wholeWordsCheckBtn);
    searchLayout->setAlignment(Qt::AlignLeft);
    searchLayout->setContentsMargins(0,10,0,0);

    QHBoxLayout *patternLayout = new QHBoxLayout();
    DLabel *patternLabel = new DLabel(QLabel::tr("File pattern:"));
    patternLabel->setFixedWidth(LABEL_WIDTH);
    patternLabel->setContentsMargins(35,0,0,0);
    d->patternLineEdit = new DLineEdit();
    d->patternLineEdit->setFixedWidth(369);
    d->patternLineEdit->setPlaceholderText(tr("e.g.*.ts,src/**/include"));
    patternLayout->addWidget(patternLabel);
    patternLayout->addWidget(d->patternLineEdit);
    patternLayout->setAlignment(Qt::AlignLeft);
    patternLayout->setContentsMargins(0,10,0,0);

    QHBoxLayout *expatternLayout = new QHBoxLayout();
    DLabel *expatternLabel = new DLabel(QLabel::tr("Exclusion pattern:"));
    expatternLabel->setFixedWidth(LABEL_WIDTH);
    d->expatternLineEdit = new DLineEdit();
    d->expatternLineEdit->setFixedWidth(369);
    d->expatternLineEdit->setPlaceholderText(tr("e.g.*.ts,src/**/include"));
    expatternLayout->addWidget(expatternLabel);
    expatternLayout->addWidget(d->expatternLineEdit);
    expatternLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    expatternLayout->setContentsMargins(0,10,0,0);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    DPushButton *searchBtn = new DPushButton(QPushButton::tr("Search"));
    searchBtn->setFixedSize(120,36);

    DPushButton *replaceBtn = new DPushButton(QPushButton::tr("Search && Replace"));
    replaceBtn->setFixedSize(155,36);
    btnLayout->addWidget(searchBtn);
    btnLayout->addWidget(replaceBtn);
    btnLayout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    btnLayout->setContentsMargins(0,5,0,0);

    connect(searchBtn, &QAbstractButton::clicked, this, &FindToolWindow::search);
    connect(replaceBtn, &QAbstractButton::clicked, this, &FindToolWindow::replace);

    vLayout->addLayout(scopeLayout);
    vLayout->addLayout(searchLayout);
    vLayout->addLayout(patternLayout);
    vLayout->addLayout(expatternLayout);
    vLayout->addLayout(btnLayout);


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
