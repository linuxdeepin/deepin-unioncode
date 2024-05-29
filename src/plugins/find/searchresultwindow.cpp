// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchresultwindow.h"
#include "common/common.h"
#include "qobjectdefs.h"
#include "base/baseitemdelegate.h"

#include <DPushButton>
#include <DMessageBox>
#include <DLabel>
#include <DLineEdit>
#include <DTreeView>
#include <DIconButton>
#include <DDialog>

#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QProcess>
#include <QDebug>
#include <QtConcurrent>
#include <QPalette>

ItemProxy::ItemProxy(QObject *parent)
    : QObject(parent)
{
}

void ItemProxy::setRuningState(bool isRuning)
{
    this->isRuning = isRuning;
}

void ItemProxy::addTask(const FindItemList &itemList)
{
    if (!isRuning)
        return;

    QHash<QString, QList<QPair<int, QString>>> findItemHash;
    for (const FindItem &findItem : itemList) {
        if (!isRuning)
            return;

        QString key = findItem.filePathName;
        auto value = qMakePair(findItem.lineNumber, findItem.context);
        if (findItemHash.contains(key)) {
            QList<QPair<int, QString>> valueList = findItemHash.value(key);
            valueList.append(value);
            findItemHash[key] = valueList;
        } else {
            findItemHash.insert(key, { value });
        }
    }

    QList<QStandardItem *> viewItemList;
    auto iter = findItemHash.begin();
    for (; iter != findItemHash.end(); ++iter) {
        if (!isRuning) {
            qDeleteAll(viewItemList);
            return;
        }

        QList<QPair<int, QString>> contentList = iter.value();
        QStandardItem *parentItem = new QStandardItem(iter.key() + " (" + QString::number(contentList.count()) + ")");
        parentItem->setData(iter.key());
        parentItem->setEditable(false);
        viewItemList << parentItem;
        for (const auto &content : contentList) {
            if (!isRuning) {
                qDeleteAll(viewItemList);
                return;
            }

            QString title = QString::number(content.first) + " " + content.second;
            QStandardItem *childItem = new QStandardItem(title);
            childItem->setEditable(false);
            int lineNumber = content.first;
            childItem->setData(lineNumber);
            parentItem->appendRow(childItem);
        }
    }

    Q_EMIT taskCompleted(viewItemList);
}

class SearchResultTreeViewPrivate
{
    SearchResultTreeViewPrivate() {}
    ~SearchResultTreeViewPrivate();

    QMap<QString, QString> projectInfoMap;
    QThread thread;
    QSharedPointer<ItemProxy> proxy;
    friend class SearchResultTreeView;
};

SearchResultTreeViewPrivate::~SearchResultTreeViewPrivate()
{
    proxy->setRuningState(false);
    thread.quit();
    thread.wait();
}

SearchResultTreeView::SearchResultTreeView(QWidget *parent)
    : DTreeView(parent), d(new SearchResultTreeViewPrivate())
{
    QAbstractItemModel *itemModel = new QStandardItemModel(this);
    setModel(itemModel);

    QObject::connect(this, &DTreeView::doubleClicked, [=](const QModelIndex &index) {
        if (!index.isValid())
            return;
        if (!index.parent().isValid())
            return;
        QModelIndex parentIndex = index.parent();
        QString filePath = parentIndex.data(Qt::UserRole + 1).toString().trimmed();
        int lineNumber = index.data(Qt::UserRole + 1).toInt();
        qInfo() << filePath << lineNumber;

        foreach (QString key, d->projectInfoMap.keys()) {
            if (filePath.contains(key, Qt::CaseInsensitive)) {
                editor.gotoLine(filePath, lineNumber);
                break;
            }
        }
    });

    d->proxy.reset(new ItemProxy);
    connect(d->proxy.data(), &ItemProxy::taskCompleted, this, &SearchResultTreeView::appendItems, Qt::QueuedConnection);

    d->proxy->moveToThread(&d->thread);
    d->thread.start();
}

SearchResultTreeView::~SearchResultTreeView()
{
    delete d;
}

void SearchResultTreeView::appendData(const FindItemList &itemList, const ProjectInfo &projectInfo)
{
    d->projectInfoMap = projectInfo;
    d->proxy->setRuningState(true);
    metaObject()->invokeMethod(d->proxy.data(),
                               "addTask",
                               Qt::QueuedConnection,
                               Q_ARG(FindItemList, itemList));
}

QIcon SearchResultTreeView::icon(const QString &data)
{
    QFileInfo info(data);
    return iconProvider.icon(info);
}

void SearchResultTreeView::appendItems(const QList<QStandardItem *> &itemList)
{
    auto model = qobject_cast<QStandardItemModel *>(SearchResultTreeView::model());
    if (!model)
        return;

    for (auto item : itemList) {
        item->setIcon(icon(item->data().toString()));
        model->appendRow(item);
    }
}

void SearchResultTreeView::clearData()
{
    d->proxy->setRuningState(false);
    auto model = qobject_cast<QStandardItemModel *>(SearchResultTreeView::model());
    model->clear();
}

class SearchResultWindowPrivate
{
    SearchResultWindowPrivate() {}
    SearchResultTreeView *treeView { nullptr };
    QWidget *replaceWidget { nullptr };
    DLineEdit *replaceEdit { nullptr };
    DLabel *resultLabel { nullptr };
    QLabel *iconLabel { nullptr };
    DDialog *replaceTextDialog { nullptr };
    DDialog *replaceWarningDialog { nullptr };
    DPushButton *replaceBtn { nullptr };
    bool replaceTextFlag = false;
    SearchParams searchParams;
    int resultCount { 0 };

    friend class SearchResultWindow;
};

SearchResultWindow::SearchResultWindow(QWidget *parent)
    : QWidget(parent), d(new SearchResultWindowPrivate())
{
    setupUi();

    qRegisterMetaType<FindItemList>("FindItemList");
    qRegisterMetaType<ProjectInfo>("ProjectInfo");
}

SearchResultWindow::~SearchResultWindow()
{
    delete d;
}

void SearchResultWindow::clear()
{
    d->iconLabel->setVisible(true);
    d->treeView->clearData();
}

void SearchResultWindow::setupUi()
{
    d->replaceWidget = new QWidget(this);
    QHBoxLayout *replaceLayout = new QHBoxLayout();

    d->replaceEdit = new DLineEdit(this);
    d->replaceEdit->setFixedWidth(280);
    d->replaceEdit->setPlaceholderText(tr("Replace"));
    d->replaceBtn = new DPushButton(DPushButton::tr("Replace"), this);
    d->replaceBtn->setFixedSize(120, 36);
    d->replaceWidget->setLayout(replaceLayout);

    replaceLayout->addWidget(d->replaceEdit, 0, Qt::AlignLeft);
    replaceLayout->addWidget(d->replaceBtn, 0, Qt::AlignLeft);
    replaceLayout->setAlignment(Qt::AlignLeft);

    QHBoxLayout *hLayout = new QHBoxLayout();
    DIconButton *cleanBtn = new DIconButton(this);   //Clean && Return
    cleanBtn->setIcon(QIcon::fromTheme("go-previous"));
    QSize iconSize(12, 12);
    cleanBtn->setIconSize(iconSize);

    cleanBtn->setFixedSize(36, 36);
    d->resultLabel = new DLabel(this);

    hLayout->addWidget(cleanBtn);
    hLayout->addWidget(d->replaceWidget);
    hLayout->addWidget(d->resultLabel);
    hLayout->setAlignment(Qt::AlignLeft);

    d->treeView = new SearchResultTreeView(this);
    d->treeView->setHeaderHidden(true);
    d->treeView->setLineWidth(0);
    d->treeView->setItemDelegate(new BaseItemDelegate(this));

    QVBoxLayout *vLayout = new QVBoxLayout();
    // vLayout->setAlignment(Qt::AlignTop);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(d->treeView, 1);

    d->iconLabel = new QLabel(this);
    QVBoxLayout *iconLayout = new QVBoxLayout();
    d->iconLabel->setPixmap(QIcon::fromTheme("find_noResults").pixmap(QSize(96, 96)));
    iconLayout->addWidget(d->iconLabel, Qt::AlignCenter);

    iconLayout->setAlignment(Qt::AlignCenter);
    vLayout->addLayout(iconLayout);
    vLayout->setAlignment(iconLayout, Qt::AlignCenter);

    connect(cleanBtn, &DIconButton::clicked, this, &SearchResultWindow::clean);
    connect(d->replaceBtn, &DPushButton::clicked, this, &SearchResultWindow::replace);

    setLayout(vLayout);
    setRepalceWidgtVisible(false);
}

void SearchResultWindow::setRepalceWidgtVisible(bool visible)
{
    d->replaceBtn->setEnabled(!visible);
    d->replaceWidget->setVisible(visible);
}

void SearchResultWindow::appendResults(const FindItemList &itemList, const ProjectInfo &projectInfo)
{
    d->treeView->setVisible(true);
    d->iconLabel->setVisible(false);
    d->treeView->appendData(itemList, projectInfo);
    d->resultCount += itemList.count();
    QString msg = tr("%1 matches found.").arg(d->resultCount);
    showMsg(true, msg);
}

void SearchResultWindow::searchFinished()
{
    if (d->resultCount > 0) {
        QString msg = tr("Search completed, %1 matches found.").arg(d->resultCount);
        showMsg(true, msg);
        d->replaceBtn->setEnabled(true);
        return;
    }

    d->treeView->setVisible(false);
    d->iconLabel->setVisible(true);
    showMsg(false, tr("No match found!"));
}

void SearchResultWindow::replaceFinished(bool success)
{
    QString msg = success ? tr("Replacement successful!") : tr("Replace failed!");
    showMsg(success, msg);
}

void SearchResultWindow::clean()
{
    d->resultCount = 0;
    d->treeView->clearData();
    emit reqBack();
}

void SearchResultWindow::replace()
{
    showMsg(true, tr("Replacing, please wait..."));
    QString replaceText = d->replaceEdit->text();
    if (replaceText.isEmpty()) {
        d->replaceTextFlag = false;
        d->replaceTextDialog = new DDialog(this);
        d->replaceTextDialog->setIcon(QIcon::fromTheme("dialog-warning"));
        d->replaceTextDialog->setMessage(tr("Repalce text is empty, will continue?"));
        d->replaceTextDialog->insertButton(0, tr("No"));
        d->replaceTextDialog->insertButton(1, tr("Yes"), true, DDialog::ButtonRecommend);

        connect(d->replaceTextDialog, &DDialog::buttonClicked, this, [=](int index) {
            if (index == 0) {
                d->replaceTextFlag = true;
                d->replaceTextDialog->reject();
            } else if (index == 1) {
                d->replaceTextDialog->accept();
            }
        });
        d->replaceTextDialog->exec();
    }

    if (d->replaceTextFlag)
        return;

    Q_EMIT reqReplace(replaceText);
}

void SearchResultWindow::showMsg(bool succeed, QString msg)
{
    QPalette palette = d->resultLabel->palette();
    QColor textColor = this->palette().color(QPalette::WindowText);

    int red, green, blue, alpha;
    textColor.getRgb(&red, &green, &blue, &alpha);
    // 降低透明度
    alpha = static_cast<int>(alpha * 0.5);   // 0.5 表示减少50%

    QColor newColor = QColor::fromRgb(red, green, blue, alpha);

    palette.setColor(QPalette::WindowText, newColor);
    d->resultLabel->setPalette(palette);
    d->resultLabel->setText(msg);
}
