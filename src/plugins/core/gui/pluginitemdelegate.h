// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef PLUGINITEMDELEGATE_H
#define PLUGINITEMDELEGATE_H

#include <DStyledItemDelegate>

class PluginItemDelegate : public DTK_WIDGET_NAMESPACE::DStyledItemDelegate
{
public:
    static inline Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment)
    {
        if (!(alignment & Qt::AlignHorizontal_Mask))
            alignment |= Qt::AlignLeft;
        if (!(alignment & Qt::AlignAbsolute) && (alignment & (Qt::AlignLeft | Qt::AlignRight))) {
            if (direction == Qt::RightToLeft)
                alignment ^= (Qt::AlignLeft | Qt::AlignRight);
            alignment |= Qt::AlignAbsolute;
        }
        return alignment;
    }
    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio,
                                 QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);

    explicit PluginItemDelegate(QAbstractItemView *parent = nullptr);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    void paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const;
    QRectF paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;
    void paintItemColumn(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index, const QRectF &iconRect) const;
};

#endif // PLUGINITEMDELEGATE_H
