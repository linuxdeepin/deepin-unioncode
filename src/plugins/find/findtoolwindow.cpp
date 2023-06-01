/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "findtoolwindow.h"
#include "searchresultwindow.h"
#include "transceiver/findreceiver.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDebug>
#include <QStackedWidget>
#include <QProcess>
#include <QMessageBox>
#include <QScrollArea>

#define LABEL_WIDTH (150)

class FindToolWindowPrivate
{
    FindToolWindowPrivate(){}
    QStackedWidget *stackedWidget{nullptr};
    SearchResultWindow *searchResultWindow{nullptr};
    QSet<QString> allProjectsPathList{nullptr};
    QString currentProjectPath;
    QString currentFilePath;
    QMap<QString, QString> projectInfoMap;

    QComboBox *scopeComboBox{nullptr};
    QLineEdit *searchLineEdit{nullptr};
    QCheckBox *senseCheckBox{nullptr};
    QCheckBox *wholeWordsCheckBox{nullptr};
    QCheckBox *regularCheckBox{nullptr};
    QLineEdit *patternLineEdit{nullptr};
    QLineEdit *expatternLineEdit{nullptr};

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
    d->stackedWidget = new QStackedWidget();
    QVBoxLayout *vLayout = new QVBoxLayout();

    QScrollArea *scrollArea = new QScrollArea();
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

    QVBoxLayout *vLayout = new QVBoxLayout();
    widget->setLayout(vLayout);

    QHBoxLayout *scopeLayout = new QHBoxLayout();
    QLabel *scopeLabel = new QLabel(QLabel::tr("Scope:"));
    scopeLabel->setAlignment(Qt::AlignRight);
    scopeLabel->setFixedWidth(LABEL_WIDTH);
    d->scopeComboBox = new QComboBox();
    d->scopeComboBox->addItem(tr("All Projects"));
    d->scopeComboBox->addItem(tr("Current Project"));
    d->scopeComboBox->addItem(tr("Current File"));
    scopeLayout->addWidget(scopeLabel);
    scopeLayout->addWidget(d->scopeComboBox);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *searchLabel = new QLabel(QLabel::tr("Search for:"));
    searchLabel->setAlignment(Qt::AlignRight);
    searchLabel->setFixedWidth(LABEL_WIDTH);
    d->searchLineEdit = new QLineEdit();
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(d->searchLineEdit);

    QHBoxLayout *ruleLayout = new QHBoxLayout();
    d->senseCheckBox = new QCheckBox(QCheckBox::tr("Case sensitive"));
    d->wholeWordsCheckBox = new QCheckBox(QCheckBox::tr("Whole words only"));
    d->regularCheckBox = new QCheckBox(QCheckBox::tr("Use regular expressions"));
    ruleLayout->addStretch();
    ruleLayout->addWidget(d->senseCheckBox);
    ruleLayout->addWidget(d->wholeWordsCheckBox);
    ruleLayout->addWidget(d->regularCheckBox);
    d->regularCheckBox->setVisible(false);

    QHBoxLayout *patternLayout = new QHBoxLayout();
    QLabel *patternLabel = new QLabel(QLabel::tr("File pattern:"));
    patternLabel->setAlignment(Qt::AlignRight);
    patternLabel->setFixedWidth(LABEL_WIDTH);
    d->patternLineEdit = new QLineEdit();
    patternLayout->addWidget(patternLabel);
    patternLayout->addWidget(d->patternLineEdit);

    QHBoxLayout *expatternLayout = new QHBoxLayout();
    QLabel *expatternLabel = new QLabel(QLabel::tr("Exclusion pattern:"));
    expatternLabel->setAlignment(Qt::AlignRight);
    expatternLabel->setFixedWidth(LABEL_WIDTH);
    d->expatternLineEdit = new QLineEdit();
    expatternLayout->addWidget(expatternLabel);
    expatternLayout->addWidget(d->expatternLineEdit);

    QPushButton *searchBtn = new QPushButton(QPushButton::tr("Search"));
    QPushButton *replaceBtn = new QPushButton(QPushButton::tr("Search && Replace"));
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(searchBtn);
    btnLayout->addWidget(replaceBtn);

    connect(searchBtn, &QAbstractButton::clicked, this, &FindToolWindow::search);
    connect(replaceBtn, &QAbstractButton::clicked, this, &FindToolWindow::replace);

    vLayout->addLayout(scopeLayout);
    vLayout->addLayout(searchLayout);
    vLayout->addLayout(ruleLayout);
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
            QMessageBox::warning(this, tr("Error"), tr("All projects path is empty, please import!"));
            return false;
        }
        break;
    }
    case 1:
    {
        if (d->currentProjectPath.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Current project path is empty, please import!"));
            return false;
        }
        break;
    }
    case 2:
    {
        if (d->currentFilePath.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Current file path is empty, please import!"));
            return false;
        }
        break;
    }
    default:
    {
        QMessageBox::warning(this, tr("Error"), tr("Scope is not selected, please select!"));
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
        QMessageBox::warning(this, tr("Error"), tr("Search for text is empty, please input!"));
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
    searchParams->sensitiveFlag = d->senseCheckBox->isChecked();
    searchParams->wholeWordsFlag = d->wholeWordsCheckBox->isChecked();
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
