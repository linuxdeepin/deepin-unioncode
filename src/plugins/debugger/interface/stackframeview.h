// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASETREEVIEW_H
#define BASETREEVIEW_H

#include <DTreeView>

#include <QHeaderView>

class StackFrameView : public DTK_WIDGET_NAMESPACE::DTreeView
{
    Q_OBJECT
public:
    explicit StackFrameView(QWidget *parent = nullptr);
    ~StackFrameView() override;

private:
    QModelIndex eventIndex(QMouseEvent *ev);
    void rowActivated(const QModelIndex &index);

    void initHeaderView();

    QHeaderView *headerView {nullptr};
};


#endif // BASETREEVIEW_H
