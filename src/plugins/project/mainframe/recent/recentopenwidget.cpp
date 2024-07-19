// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentopenwidget.h"
#include "common/util/customicons.h"

#include <DStyle>

#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QFileInfo>

DWIDGET_USE_NAMESPACE

RecentOpenWidget::RecentOpenWidget(QWidget *parent)
    : DFrame(parent)
{
    initUI();
}

RecentOpenWidget::~RecentOpenWidget()
{
}

void RecentOpenWidget::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);

    listView = new RecentOpenView(this);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listView->setLineWidth(0);
    listView->setHeaderHidden(true);
    listView->setContentsMargins(0, 0, 0, 0);
    listView->setRootIsDecorated(false);
    listView->setIconSize(QSize(16, 16));
    delegate = new RecentOpenItemDelegate(listView);
    listView->setItemDelegate(delegate);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);

    model = new QStandardItemModel(this);
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setSortRole(Qt::DisplayRole);
    proxyModel->sort(0);
    listView->setModel(proxyModel);

    connect(listView, &RecentOpenView::clicked, this, &RecentOpenWidget::triggered);
    connect(listView, &RecentOpenView::closeActivated, this, &RecentOpenWidget::closePage);
    connect(delegate, &RecentOpenItemDelegate::closeBtnClicked, this, &RecentOpenWidget::closePage);

    mainLayout->addWidget(listView);
    this->setLineWidth(0);
    this->setLayout(mainLayout);
    DStyle::setFrameRadius(this, 0);
}

void RecentOpenWidget::setOpenedFiles(const QVector<QString> &list)
{
    model->clear();
    QSet<QString> fileNamesSet;

    for (const QString &absolutePath : list) {
        QFileInfo info(absolutePath);
        QString fileName = info.fileName();
        QStandardItem *item = new QStandardItem(fileName);
        item->setData(absolutePath, RecentOpenedUserRole::FilePathRole);
        item->setIcon(CustomIcons::icon(info));
        model->appendRow(item);
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem *item = model->item(i);
        QString fileName = item->text();
        if (fileNamesSet.contains(fileName)) {
            for (int j = 0; j < model->rowCount(); ++j) {
                QStandardItem *nextItem = model->item(j);
                if (nextItem->text() == fileName) {
                    QString dirName = QFileInfo(nextItem->data(RecentOpenedUserRole::FilePathRole).toString()).absolutePath();
                    dirName = dirName.mid(dirName.lastIndexOf('/') + 1);
                    QString newName = dirName + "/" + fileName;
                    nextItem->setText(newName);
                }
            }
        } else {
            fileNamesSet.insert(fileName);
        }
    }
}

void RecentOpenWidget::setListViewSelection(int num)
{
    if (num < 0 || num >= proxyModel->rowCount() || !listView || !proxyModel) {
        return;
    }

    QModelIndex index = proxyModel->index(num, 0);
    listView->setCurrentIndex(index);
}

void RecentOpenWidget::setListViewSelection(const QString &file)
{
    if (!listView || !proxyModel) {
        return;
    }
    int rowCount = proxyModel->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QModelIndex proxyIndex = proxyModel->index(i, 0);
        QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
        if (file == model->itemData(sourceIndex).value(RecentOpenedUserRole::FilePathRole)) {  
            listView->setCurrentIndex(proxyIndex);
            break;
        }
    }
}

void RecentOpenWidget::setFocusListView()
{
    listView->setFocus();
}
