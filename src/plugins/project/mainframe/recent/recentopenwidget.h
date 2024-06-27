// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTOPENWIDGET_H
#define RECENTOPENWIDGET_H

#include "recentopenlistview.h"
#include "recentopenlistdelegate.h"

#include <DFrame>

#include <QKeyEvent>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class RecentOpenWidget : public Dtk::Widget::DFrame
{
    Q_OBJECT

public:
    enum RecentOpenedUserRole{
        FilePathRole = Qt::UserRole + 1
    };
    explicit RecentOpenWidget(QWidget *parent = nullptr);
    ~RecentOpenWidget();
    
    void initUI();
    void setOpenedFiles(const QVector<QString> &list);
    void setListViewSelection(int index);
    void setListViewSelection(const QString &file);
    void setFocusListView();

signals:
    void triggered(const QModelIndex &index);
    void closePage(const QModelIndex &index);

private:
    RecentOpenListView *listView = nullptr;
    QStandardItemModel *model = nullptr;
    QSortFilterProxyModel *proxyModel = nullptr;
    RecentOpenListDelegate *delegate = nullptr;
};

#endif // RECENTOPENWIDGET_H
