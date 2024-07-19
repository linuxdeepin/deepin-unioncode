// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTOPENVIEW_H
#define RECENTOPENVIEW_H

#include <DTreeView>

class RecentOpenView : public Dtk::Widget::DTreeView
{
    Q_OBJECT
public:
    explicit RecentOpenView(QWidget *parent = nullptr);
    ~RecentOpenView();

signals:
    void closeActivated(const QModelIndex &index);

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // RECENTOPENVIEW_H
