#ifndef DISPLAYITEMDELEGATE_H
#define DISPLAYITEMDELEGATE_H

#include <QStyledItemDelegate>

class DisplayItemDelegate : public QStyledItemDelegate
{
public:
    explicit DisplayItemDelegate(QObject *parent = nullptr);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

#endif // DISPLAYITEMDELEGATE_H
