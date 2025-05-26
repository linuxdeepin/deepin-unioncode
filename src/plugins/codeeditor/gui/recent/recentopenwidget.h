// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTOPENWIDGET_H
#define RECENTOPENWIDGET_H

#include "recentopenlistview.h"
#include "recentopenlistdelegate.h"

#include <DFrame>

#include <QKeyEvent>
#include <QStandardItemModel>

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
    void setFocusListView();

signals:
    void triggered(const QModelIndex &index);
    
protected:
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    RecentOpenListView *listView = nullptr;
    QStandardItemModel *model = nullptr;
    RecentOpenListDelegate *delegate = nullptr;
};

#endif // RECENTOPENWIDGET_H
