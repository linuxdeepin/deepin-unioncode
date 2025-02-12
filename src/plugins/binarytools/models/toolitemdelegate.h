// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLITEMDELEGATE_H
#define TOOLITEMDELEGATE_H

#include <DStyledItemDelegate>

class QTreeView;
class ToolItemDelegate : public DTK_WIDGET_NAMESPACE::DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ToolItemDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    QTreeView *view() const;
    void drawBackground(QPainter *painter, const QStyleOptionViewItem &option) const;
    void drawToolItem(QPainter *painter,
                      const QStyleOptionViewItem &option,
                      const QModelIndex &index) const;
    QRect drawItemIcon(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    QRect drawExpandArrow(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    QRect arrowRect(const QRect &itemRect) const;
};

#endif   // TOOLITEMDELEGATE_H
