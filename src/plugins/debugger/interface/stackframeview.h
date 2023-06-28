// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASETREEVIEW_H
#define BASETREEVIEW_H

#include <QTreeView>

class StackFrameView : public QTreeView
{
    Q_OBJECT
public:
    explicit StackFrameView(QWidget *parent = nullptr);
    ~StackFrameView() override;

private:
    QModelIndex eventIndex(QMouseEvent *ev);
    void rowActivated(const QModelIndex &index);
    void rowClicked(const QModelIndex &index);
};


#endif // BASETREEVIEW_H
