// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESULTITEMDELEGATE_H
#define SEARCHRESULTITEMDELEGATE_H

#include <DStyledItemDelegate>
#include <QTextLayout>

class QTreeView;
class SearchResultItemDelegate : public DTK_WIDGET_NAMESPACE::DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SearchResultItemDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QTreeView *view() const;
    void drawBackground(QPainter *painter, const QStyleOptionViewItem &option) const;
    QRect drawFileIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect drawExpandArrow(QPainter *painter, const QRect &iconRect,
                          const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect drawResultCount(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect drawOptionButton(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void drawNameItem(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index, const QRect &iconRect) const;
    void drawContextItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect,
                     const QString &text, const QList<QTextLayout::FormatRange> &format) const;
    QSizeF doTextLayout(QTextLayout *textLayout, int width) const;
    QRect iconRect(const QRect &itemRect) const;
    QRect arrowRect(const QRect &iconRect) const;
    QRect replaceButtonRect(const QRect &itemRect) const;
    QRect closeButtonRect(const QRect &itemRect) const;
    QTextLayout::FormatRange createFormatRange(const QStyleOptionViewItem &option, int start, int length,
                                               const QColor &foreground, const QColor &background) const;
    void drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QIcon &icon, const QRect &rect) const;
};

#endif   // SEARCHRESULTITEMDELEGATE_H
