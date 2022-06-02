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
#include "transceiver/findsender.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QStandardItemModel>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>

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
        QString filePath = parentIndex.data(Qt::UserRole+1).toString();
        int lineNumber = index.data(Qt::UserRole+1).toInt();
        qInfo() << filePath << lineNumber;

        foreach (QString key, d->projectInfoMap.keys()) {
            if (filePath.contains(key, Qt::CaseInsensitive)) {
                FindSender::sendDoubleClickFileInfo(key, d->projectInfoMap.value(key), filePath, lineNumber);
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

class SearchResultWindowPrivate
{
    SearchResultWindowPrivate(){}
    SearchResultTreeView *treeView{nullptr};
    QWidget *replaceWidget{nullptr};
    QLineEdit *replaceEdit{nullptr};

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
    QPushButton *cleanBtn = new QPushButton(QPushButton::tr("Clean"));
    cleanBtn->setFixedHeight(30);
    hLayout->addWidget(d->replaceWidget, 0, Qt::AlignLeft);
    hLayout->addWidget(cleanBtn, 0, Qt::AlignLeft);
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
    // exam: grep -rn -i -w "main" --include="*.txt" --exclude="*.txt" /project/test
    QProcess process;
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

    process.start("grep -rn " + sensitiveFlag + wholeWordsFlag
                  + "\"" + params->searchText + "\" "
                  + patternList + exPatternList + " " + filePath);
    process.waitForFinished();
    QString output = QString(process.readAllStandardOutput());
    QStringList outputList = output.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);

    FindItemList findItemList;

    QSet<QString> resultFilePathSet;
    foreach (QString line, outputList) {
        QStringList contentList = line.split(":");
        if (contentList.count() == 2) {
            FindItem findItem;
            findItem.filePathName = filePath;
            findItem.lineNumber = contentList[0].toInt();
            findItem.context = contentList[1];
            findItem.projectInfoMap = params->projectInfoMap;
            findItemList.append(findItem);
            resultFilePathSet.insert(filePath);
        } else if (contentList.count() == 3) {
            FindItem findItem;
            findItem.filePathName = contentList[0];
            findItem.lineNumber = contentList[1].toInt();
            findItem.context = contentList[2];
            findItem.projectInfoMap = params->projectInfoMap;
            findItemList.append(findItem);
            resultFilePathSet.insert(filePath);
        }
    }

    d->searchParams.filePathList = resultFilePathSet.toList();
    d->searchParams.searchText = params->searchText;
    d->searchParams.sensitiveFlag = params->sensitiveFlag;
    d->searchParams.wholeWordsFlag = params->wholeWordsFlag;
    d->searchParams.patternsList = params->patternsList;
    d->searchParams.exPatternsList = params->exPatternsList;
    d->searchParams.projectInfoMap = params->projectInfoMap;

    d->treeView->setData(findItemList, params->projectInfoMap);
}

void SearchResultWindow::clean()
{
    emit back();
}

void SearchResultWindow::replace()
{
    QString replaceText = d->replaceEdit->text();
    if (replaceText.isEmpty()) {
        if (QMessageBox::Yes != QMessageBox::warning(this, tr("Warning"), tr("Repalce text is empty, will continue?"),
                                                     QMessageBox::Yes, QMessageBox::No)) {
            return;
        }
    }

    if (QMessageBox::Yes != QMessageBox::warning(this, tr("Warning"), tr("Will replace permanent, continue?"),
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

    QProcess process;
    process.start("/bin/sh", options);
    process.waitForFinished();
    QString output = QString(process.readAllStandardOutput());

    search(&d->searchParams);
}
