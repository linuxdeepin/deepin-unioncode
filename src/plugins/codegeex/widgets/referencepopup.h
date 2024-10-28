// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REFERENCESPOPUP_H
#define REFERENCESPOPUP_H

#include <DListView>
#include <DBlurEffectWidget>

#include <QAbstractListModel>

DWIDGET_USE_NAMESPACE

class DisplayItemDelegate : public DStyledItemDelegate
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

    explicit DisplayItemDelegate(QAbstractItemView *parent = nullptr);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    void paintItemBackground(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const;
    void paintItemColumn(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;
};

struct ItemInfo
{
    QString type { "" };
    QString displayName { "" };
    QString extraInfo { "" };
    QIcon icon;
};

class ItemModel : public QAbstractListModel
{
public:
    enum Columns {
        DisplayNameColumn,
        ExtraInfoColumn,
        ColumnCount
    };

    ItemModel(QObject *parent = nullptr)
        : QAbstractListModel(parent) {}
    void clear();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayPropertyRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void addItems(const QList<ItemInfo> &items);
    void setFilterText(const QString &filterText);

    QList<ItemInfo> getItems() const;

private:
    QList<ItemInfo> items;
    QList<ItemInfo> displayItems;
};

class PopupWidget : public DBlurEffectWidget
{
    Q_OBJECT
public:
    PopupWidget(QWidget *parent = nullptr);
    void setmodel(ItemModel *model);
    void keyPressEvent(QKeyEvent *event) override;
    void selectFirstRow();

signals:
    void selectIndex(const QModelIndex &index);

protected:
    void updateGeometry();

    void next();
    void previous();

    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    DListView *list { nullptr };
    ItemModel *model { nullptr };
};

#endif   // REFERENCESPOPUP_H
