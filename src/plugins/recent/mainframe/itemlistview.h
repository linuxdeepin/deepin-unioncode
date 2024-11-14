// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITEMLISTVIEW_H
#define ITEMLISTVIEW_H

#include <DListView>
#include <DFileIconProvider>

DWIDGET_USE_NAMESPACE

class QStandardItemModel;
class QStandardItem;
class ItemListView : public DListView
{
    Q_OBJECT
public:
    explicit ItemListView(QWidget *parent = nullptr);
    ~ItemListView() override;

    virtual QString title() = 0;
    virtual QString configKey() = 0;
    virtual void setItemList(const QVariantList &items) = 0;
    virtual QVariantList itemList() const = 0;
    virtual void appendItem(const QVariant &item) = 0;
    virtual void prependItem(const QVariant &item) = 0;
    virtual void clear();

    void focusOutEvent(QFocusEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

protected:
    QStandardItemModel *model { nullptr };
};

#endif   // ITEMLISTVIEW_H
