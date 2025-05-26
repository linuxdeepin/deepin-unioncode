// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTOPENLISTVIEW_H
#define RECENTOPENLISTVIEW_H

#include <DListView>

class RecentOpenListView : public Dtk::Widget::DListView
{
    Q_OBJECT
public:
    explicit RecentOpenListView(QWidget *parent = nullptr);
    ~RecentOpenListView();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // RECENTOPENLISTVIEW_H
