// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchresultwindow.h"
#include "common/common.h"

#include <DPushButton>
#include <DMessageBox>
#include <DLabel>
#include <DLineEdit>
#include <DTreeView>
#include <DIconButton>

#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QProcess>
#include <QDebug>
#include <QtConcurrent>
#include <QPalette>

class SearchResultTreeViewPrivate
{
    SearchResultTreeViewPrivate() {}
    QMap<QString, QString> projectInfoMap;

    friend class SearchResultTreeView;
};

SearchResultTreeView::SearchResultTreeView(QWidget *parent)
    : DTreeView(parent)
    , d(new SearchResultTreeViewPrivate())
{
    QAbstractItemModel *itemModel = new QStandardItemModel();
    setModel(itemModel);

    QObject::connect(this, &DTreeView::doubleClicked, [=](const QModelIndex &index){
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
        QStandardItem *parentItem = new QStandardItem(QIcon::fromTheme("folder"),iter.key() + " (" + QString::number(contentList.count()) + ")");
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
    DLineEdit *replaceEdit{nullptr};
    DLabel *resultLabel{nullptr};
    QLabel *iconLabel{nullptr};

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

    d->replaceEdit = new DLineEdit();
    d->replaceEdit->setFixedWidth(280);
    d->replaceEdit->setPlaceholderText(tr("Replace"));
    DPushButton *replaceBtn = new DPushButton(DPushButton::tr("Replace"));
    replaceBtn->setFixedSize(120,36);
    d->replaceWidget->setLayout(replaceLayout);

    replaceLayout->addWidget(d->replaceEdit, 0, Qt::AlignLeft);
    replaceLayout->addWidget(replaceBtn, 0, Qt::AlignLeft);
    replaceLayout->setAlignment(Qt::AlignLeft);

    QHBoxLayout *hLayout = new QHBoxLayout();
    DIconButton *cleanBtn = new DIconButton();//Clean && Return
    cleanBtn->setIcon(QIcon::fromTheme("go-previous"));
    QSize iconSize(12,12);
    cleanBtn->setIconSize(iconSize);

    cleanBtn->setFixedSize(36,36);
    d->resultLabel = new DLabel();

    hLayout->addWidget(cleanBtn);
    hLayout->addWidget(d->replaceWidget);
    hLayout->addWidget(d->resultLabel);
    hLayout->setAlignment(Qt::AlignLeft );

    d->treeView = new SearchResultTreeView();
    d->treeView->setHeaderHidden(true);
    d->treeView->setLineWidth(0);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setAlignment(Qt::AlignTop);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(d->treeView);

    d->iconLabel =new QLabel();
    QVBoxLayout *iconLayout = new QVBoxLayout();
    d->iconLabel->setPixmap(QIcon::fromTheme("find_noResults").pixmap(QSize(96, 96)));
    iconLayout->addWidget(d->iconLabel, Qt::AlignCenter);

    iconLayout->setAlignment(Qt::AlignCenter);
    vLayout->addLayout(iconLayout);
    vLayout->setAlignment(iconLayout, Qt::AlignCenter);

    connect(this, &SearchResultWindow::searched, this,[=]{
        d->treeView->setVisible(false);
        d->iconLabel->setVisible(false);
    });
    connect(this, &SearchResultWindow::replaced, this,[=]{
        d->treeView->setVisible(false);
        d->iconLabel->setVisible(false);
    });
    connect(cleanBtn, &DIconButton::clicked, this, &SearchResultWindow::clean);
    connect(replaceBtn, &DPushButton::clicked, this, &SearchResultWindow::replace);
    connect(this, &SearchResultWindow::haveResult, this,[=]{
        d->treeView->setVisible(true);
        d->iconLabel->setVisible(false);
    });
    connect(this, &SearchResultWindow::noResult, this,[=]{
        d->treeView->setVisible(false);
        d->iconLabel->setVisible(true);
    });

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
    showMsg(true, tr("Searching, please wait..."));
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

    emit searched();

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
                    FindItem findItem;
                    findItem.filePathName = regMatch.captured(1).trimmed().toStdString().c_str();
                    findItem.lineNumber = regMatch.captured(2).trimmed().toInt();
                    findItem.context = regMatch.captured(3).trimmed().toStdString().c_str();
                    findItemList.append(findItem);
                    resultCount++;
                } else if ((regMatch = IN_FILE_REG.match(line)).hasMatch()) {
                    FindItem findItem;
                    findItem.filePathName = filePath;
                    findItem.lineNumber = regMatch.captured(1).trimmed().toInt();
                    findItem.context = regMatch.captured(2).trimmed().toStdString().c_str();
                    findItemList.append(findItem);
                    resultCount++;
                }
            }

            d->treeView->setData(findItemList, projectInfoMap);
            QString msg = QString::number(resultCount) + tr(" matches found.");
            showMsg(true, msg);
            emit haveResult();

        } else {
            showMsg(false, tr("Search failed!"));
            emit noResult();
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
    showMsg(true, tr("Replacing, please wait..."));
    QString replaceText = d->replaceEdit->text();
    if (replaceText.isEmpty()) {
        if (DMessageBox::Yes != DMessageBox::warning(this, DMessageBox::tr("Warning"), DMessageBox::tr("Repalce text is empty, will continue?"),
                                                      DMessageBox::Yes, DMessageBox::No)) {
            return;
        }
    }

    if (DMessageBox::Yes != DMessageBox::warning(this, DMessageBox::tr("Warning"), DMessageBox::tr("Will replace permanent, continue?"),
                                                DMessageBox::Yes, DMessageBox::No)) {
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

    emit replaced();

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
            showMsg(false, tr("Replace failed!"));
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

    QPalette palette = d->resultLabel->palette();
    QColor textColor = this->palette().color(QPalette::WindowText);

    int red, green, blue, alpha;
    textColor.getRgb(&red, &green, &blue, &alpha);
    // 降低透明度
    alpha = static_cast<int>(alpha * 0.5); // 0.5 表示减少50%

    QColor newColor = QColor::fromRgb(red, green, blue, alpha);

    palette.setColor(QPalette::WindowText, newColor);
    d->resultLabel->setPalette(palette);
    d->resultLabel->setText(msg);
}
