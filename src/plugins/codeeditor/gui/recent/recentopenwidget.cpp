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

void RecentOpenWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        if (this->listView->selectionModel()->hasSelection()) {
            this->triggered(listView->selectionModel()->selectedIndexes()[0]);
            this->close();
        }
    }
    DFrame::keyReleaseEvent(event);
}

void RecentOpenWidget::initUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout();
    
    listView = new RecentOpenListView(this);
    listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listView->setLineWidth(0);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    delegate = new RecentOpenListDelegate(listView);
    listView->setItemDelegate(delegate);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);

    model = new QStandardItemModel(this);
    listView->setModel(model);

    connect(listView, &RecentOpenListView::clicked, this, &RecentOpenWidget::triggered);

    mainLayout->addWidget(listView);
    mainLayout->setContentsMargins(10, 10, 10, 0);
    this->setLineWidth(0);
    this->setLayout(mainLayout);
    this->setMinimumSize(400, 265);
    DStyle::setFrameRadius(this, 16);
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
    if (num < 0 || !listView || !listView->model()) {
        return;  
    }

    int rowCount = listView->model()->rowCount();
    
    if (num >= rowCount) {
        return;
    }
    listView->setCurrentIndex(listView->model()->index(num, 0));
}

void RecentOpenWidget::setFocusListView()
{
    listView->setFocus();
}
