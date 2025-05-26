// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKDELEGATE_H
#define TASKDELEGATE_H

#include "taskview.h"

#include <DStyledItemDelegate>

#include <QObject>
#include <QFont>

class Positions;
class TaskDelegate : public DTK_WIDGET_NAMESPACE::DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TaskDelegate(TaskView *parent);
    ~TaskDelegate() override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    // TaskView uses this method if the size of the taskview changes
    void emitSizeHintChanged(const QModelIndex &index);

    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    void generateGradientPixmap(int width, int height, QColor color, bool selected) const;

    void paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const;
    void paintItemColumn(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index, const QRect &iconRect) const;
    QRect paintFixButton(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;

    QRect iconRect(const QRect &itemRect) const;
    QRect fixButtonRect(const QRect &itemRect) const;

    TaskView *view { nullptr };
};

#endif   // TASKDELEGATE_H
