// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentopenwidget.h"

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
    
    model = new QStandardItemModel(this);
    listView->setModel(model);
    
    connect(listView, &RecentOpenListView::clicked, this, &RecentOpenWidget::triggered);
    
    mainLayout->addWidget(listView);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(mainLayout);
    this->setFixedSize(400, 265);
    this->setLineWidth(0);
}

void RecentOpenWidget::setOpenedFiles(const QVector<QString> &list)
{
    model->clear();

    for (int i = 0; i < list.size(); ++i) {
        QString absolutePath = list.at(i);
        QString fileName = QFileInfo(absolutePath).fileName();
        QString dirName = QFileInfo(absolutePath).absolutePath();
        dirName = dirName.mid(dirName.lastIndexOf('/') + 1);
        fileName = dirName + "/" + fileName;
        QStandardItem *item = new QStandardItem(fileName);
        item->setData(absolutePath, RecentOpenedUserRole::FilePathRole);
        model->appendRow(item);
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
