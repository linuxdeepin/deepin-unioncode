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
#include "searchresultwindow.h"
#include "common/common.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QStandardItemModel>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QtConcurrent>

class SearchResultTreeViewPrivate
{
    SearchResultTreeViewPrivate() {}
    QMap<QString, QString> projectInfoMap;

    friend class SearchResultTreeView;
};

SearchResultTreeView::SearchResultTreeView(QWidget *parent)
    : QTreeView(parent)
    , d(new SearchResultTreeViewPrivate())
{
    QAbstractItemModel *itemModel = new QStandardItemModel();
    setModel(itemModel);

    QObject::connect(this, &QTreeView::doubleClicked, [=](const QModelIndex &index){
        if (!index.isValid())
            return;
        if (!index.parent().isValid())
            return;
        QModelIndex parentIndex = index.parent();
        QString filePath = parentIndex.data(Qt::UserRole+1).toString().trimmed();
        int lineNumber = index.data(Qt::UserRole+1).toInt();
        qInfo() << filePath << lineNumber;

        foreach (QString key, d->projectInfoMap.keys()) {
            if (filePath.contains(key, Qt::CaseInsensitive)) {
                editor.jumpToLineWithKey(key, d->projectInfoMap.value(key), filePath, lineNumber);
                break;
            }
        }
    });
}

void SearchResultTreeView::setData(FindItemList &itemList, QMap<QString, QString> projectInfoMap)
{
    auto model = qobject_cast<QStandardItemModel*>(SearchResultTreeView::model());
    model->clear();
    d->projectInfoMap = projectInfoMap;

    QHash<QString, QList<QPair<int, QString>>> findItemHash;
    for (FindItem findItem : itemList) {
        QString key = findItem.filePathName;
        QPair<int, QString> value = QPair<int, QString>(findItem.lineNumber, findItem.context);
        if (findItemHash.contains(key)) {
            QList<QPair<int, QString>> valueList = findItemHash.value(key);
            valueList.append(value);
            findItemHash[key] = valueList;
        } else {
            findItemHash.insert(key, {value});
        }
    }

    QHash<QString, QList<QPair<int, QString>>>::const_iterator iter = findItemHash.begin();
    for (; iter != findItemHash.end(); ++iter) {
        QList<QPair<int, QString>> contentList = iter.value();
        QStandardItem *parentItem = new QStandardItem(iter.key() + " (" + QString::number(contentList.count()) + ")");
        parentItem->setData(QVariant::fromValue<QString>(iter.key()));
        parentItem->setEditable(false);
        model->appendRow(parentItem);
        for (QPair<int, QString> content : contentList) {
            QString title = QString::number(content.first) + " " + content.second;
            QStandardItem *childItem = new QStandardItem(title);
            childItem->setEditable(false);
            int lineNumber = content.first;
            childItem->setData(QVariant::fromValue<int>(lineNumber));
            parentItem->appendRow(childItem);
        }
    }
}

void SearchResultTreeView::clearData()
{
    auto model = qobject_cast<QStandardItemModel*>(SearchResultTreeView::model());
    model->clear();
}

class SearchResultWindowPrivate
{
    SearchResultWindowPrivate(){}
    SearchResultTreeView *treeView{nullptr};
    QWidget *replaceWidget{nullptr};
    QLineEdit *replaceEdit{nullptr};
    QLabel *resultLabel{nullptr};

    SearchParams searchParams;

    friend class SearchResultWindow;
};

SearchResultWindow::SearchResultWindow(QWidget *parent)
    : QWidget(parent)
    , d(new SearchResultWindowPrivate())
{
    setupUi();
}

void SearchResultWindow::setupUi()
{
    d->replaceWidget = new QWidget();
    QHBoxLayout *replaceLayout = new QHBoxLayout();
    QLabel *replaceLabel = new QLabel(QLabel::tr("Replace with:"));
    replaceLabel->setAlignment(Qt::AlignRight);
    replaceLabel->setFixedWidth(120);
    d->replaceEdit = new QLineEdit();
    d->replaceEdit->setFixedWidth(500);
    QPushButton *replaceBtn = new QPushButton(QPushButton::tr("Replace"));
    replaceBtn->setFixedHeight(30);
    d->replaceWidget->setLayout(replaceLayout);

    replaceLayout->addWidget(replaceLabel, 0, Qt::AlignRight);
    replaceLayout->addWidget(d->replaceEdit, 0, Qt::AlignLeft);
    replaceLayout->addWidget(replaceBtn, 0, Qt::AlignLeft);
    replaceLayout->addStretch();

    QHBoxLayout *hLayout = new QHBoxLayout();
    QPushButton *cleanBtn = new QPushButton(QPushButton::tr("Clean && Return"));
    cleanBtn->setFixedHeight(30);
    d->resultLabel = new QLabel();
    hLayout->addWidget(d->replaceWidget, 0, Qt::AlignLeft);
    hLayout->addWidget(cleanBtn, 0, Qt::AlignLeft);
    hLayout->addWidget(d->resultLabel, 0, Qt::AlignLeft);
    hLayout->addStretch(0);

    d->treeView = new SearchResultTreeView();
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayout);
    vLayout->addWidget(d->treeView);

    connect(cleanBtn, &QPushButton::clicked, this, &SearchResultWindow::clean);
    connect(replaceBtn, &QPushButton::clicked, this, &SearchResultWindow::replace);

    setLayout(vLayout);

    setRepalceWidgtVisible(false);
}

void SearchResultWindow::setRepalceWidgtVisible(bool visible)
{
    d->replaceWidget->setVisible(visible);
}

void SearchResultWindow::search(SearchParams *params)
{
    d->treeView->clearData();
    showMsg(true, "Searching, please wait...");
    // exam: grep -rn -i -w "main" --include="*.txt" --exclude="*.txt" /project/test
    QString filePath;
    for (QString path : params->filePathList) {
        filePath += path;
        filePath += " ";
    }

    QString sensitiveFlag = params->sensitiveFlag ? "" : " -i ";
    QString wholeWordsFlag = params->wholeWordsFlag ? " -w " : "";
    QString patternList = "";
    foreach (QString pattern, params->patternsList) {
        patternList += " --include=" + pattern;
    }
    QString exPatternList = "";
    foreach (QString expattern, params->exPatternsList) {
        exPatternList += " --exclude=" + expattern;
    }

    QString cmd = QString("grep -rn " + sensitiveFlag + wholeWordsFlag
                   + "\"" + params->searchText + "\" "
                   + patternList + exPatternList + " " + filePath);

    d->searchParams.filePathList = params->filePathList;
    d->searchParams.searchText = params->searchText;
    d->searchParams.sensitiveFlag = params->sensitiveFlag;
    d->searchParams.wholeWordsFlag = params->wholeWordsFlag;
    d->searchParams.patternsList = params->patternsList;
    d->searchParams.exPatternsList = params->exPatternsList;
    d->searchParams.projectInfoMap = params->projectInfoMap;

    QtConcurrent::run(this, &SearchResultWindow::startSearch, cmd, filePath, params->projectInfoMap);
}

void SearchResultWindow::startSearch(const QString &cmd, const QString &filePath, QMap<QString, QString> projectInfoMap)
{
    QProcess process;
    connect(&process, static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
            [&](int exitcode, QProcess::ExitStatus exitStatus) {
        if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
            QString output = QString(process.readAllStandardOutput());
            QStringList outputList = output.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
            FindItemList findItemList;
            int resultCount = 0;
            foreach (QString line, outputList) {
                //search in folder: "filepath:lineNumber:text"
                const auto IN_FOLDER_REG = QRegularExpression(R"((.+):([0-9]+):(.+))", QRegularExpression::NoPatternOption);

                //search in file: "lineNumber:text"
                const auto IN_FILE_REG = QRegularExpression(R"(([0-9]+):(.+))", QRegularExpression::NoPatternOption);

                QRegularExpressionMatch regMatch;
                if ((regMatch = IN_FOLDER_REG.match(line)).hasMatch()) {
                    qInfo() << regMatch;
                    FindItem findItem;
                    findItem.filePathName = regMatch.captured(1).trimmed().toStdString().c_str();
                    findItem.lineNumber = regMatch.captured(2).trimmed().toInt();
                    findItem.context = regMatch.captured(3).trimmed().toStdString().c_str();
                    findItemList.append(findItem);
                    resultCount++;
                } else if ((regMatch = IN_FILE_REG.match(line)).hasMatch()) {
                    qInfo() << regMatch;
                    FindItem findItem;
                    findItem.filePathName = filePath;
                    findItem.lineNumber = regMatch.captured(1).trimmed().toInt();
                    findItem.context = regMatch.captured(2).trimmed().toStdString().c_str();
                    findItemList.append(findItem);
                    resultCount++;
                }
            }
            d->treeView->setData(findItemList, projectInfoMap);
            QString msg = QString::number(resultCount) + " matches found.";
            showMsg(true, msg);
        } else {
            showMsg(false, "Search failed!");
        }
    });

    process.start(cmd);
    process.waitForFinished();
}

void SearchResultWindow::clean()
{
    d->treeView->clearData();
    emit back();
}

void SearchResultWindow::replace()
{
    d->treeView->clearData();
    showMsg(true, "Replacing, please wait...");
    QString replaceText = d->replaceEdit->text();
    if (replaceText.isEmpty()) {
        if (QMessageBox::Yes != QMessageBox::warning(this, QMessageBox::tr("Warning"), QMessageBox::tr("Repalce text is empty, will continue?"),
                                                     QMessageBox::Yes, QMessageBox::No)) {
            return;
        }
    }

    if (QMessageBox::Yes != QMessageBox::warning(this, QMessageBox::tr("Warning"), QMessageBox::tr("Will replace permanent, continue?"),
                                                 QMessageBox::Yes, QMessageBox::No)) {
        return;
    }

    QString filePath;
    for (QString path : d->searchParams.filePathList) {
        filePath += path;
        filePath += " ";
    }

    //exam: sed -i "s/main/main1/g" `grep -rl "main" /project/test`
    QString cmd = "sed -i \"s/" + d->searchParams.searchText
            + "/" + replaceText + "/g\" `grep -rl \"" + d->searchParams.searchText
            + "\" " + filePath + "`";
    QStringList options;
    options << "-c" << cmd;

    QtConcurrent::run(this, &SearchResultWindow::startReplace, options);
}

void SearchResultWindow::startReplace(const QStringList &options)
{
    QProcess process;
    connect(&process, static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),
            [&](int exitcode, QProcess::ExitStatus exitStatus) {
        if (0 == exitcode && exitStatus == QProcess::ExitStatus::NormalExit) {
            QString output = QString(process.readAllStandardOutput());
            searchAgain();
        } else {
            showMsg(false, "Replace failed!");
        }
    });

    process.start("/bin/sh", options);
    process.waitForFinished();
}

void SearchResultWindow::searchAgain()
{
    search(&d->searchParams);
}

void SearchResultWindow::showMsg(bool succeed, QString msg)
{
    if (succeed) {
        d->resultLabel->setStyleSheet("color:white;");
    } else {
        d->resultLabel->setStyleSheet("color:red;");
    }
    d->resultLabel->setText(msg);
}
