// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTITEMDELEGATE_H
#define PROJECTITEMDELEGATE_H

#include "gui/projecttreeview.h"

#include "common/util/spinnerpainter.h"

#include <DStyledItemDelegate>

class ProjectItemDelegate : public DTK_WIDGET_NAMESPACE::DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ProjectItemDelegate(ProjectTreeView *parent);
    ~ProjectItemDelegate();

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
    bool helpEvent(QHelpEvent *event,
                   QAbstractItemView *view,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) override;

private:
    void drawBackground(QPainter *painter, const QStyleOptionViewItem &option) const;
    QRect drawFileIcon(int depth,
                       QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    QRect drawExpandArrow(int depth,
                          QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void drawCheckBox(int depth,
                      QPainter *painter,
                      const QStyleOptionViewItem &option,
                      const QModelIndex &index) const;
    QRect drawItemState(QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;
    void drawFileNameItem(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index,
                          const QRect &iconRect) const;
    void drawIcon(QPainter *painter, const QStyleOptionViewItem &option,
                  const QIcon &icon, const QRect &rect) const;

    QRect checkBoxRect(int depth, const QRect &itemRect) const;
    QRect fileIconRect(int depth, const QRect &itemRect, const QModelIndex &index) const;
    QRect arrowRect(int depth, const QRect &itemRect) const;
    QRect itemStateRect(const QRect &itemRect) const;

    int itemDepth(const QModelIndex &index) const;
    SpinnerPainter *findOrCreateSpinnerPainter(const QModelIndex &index) const;
    void updateChildrenCheckState(QAbstractItemModel *model,
                                  const QModelIndex &index,
                                  Qt::CheckState state);
    void updateParentCheckState(QAbstractItemModel *model, const QModelIndex &index);

private:
    ProjectTreeView *view { nullptr };
    mutable QHash<QModelIndex, SpinnerPainter *> spinners;
};

#endif   // PROJECTITEMDELEGATE_H
