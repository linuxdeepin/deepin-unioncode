// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef POPUPWIDGET_H
#define POPUPWIDGET_H

#include "base/abstractlocator.h"

#include <DWidget>
#include <DTreeView>
#include <DFrame>

#include <QAbstractListModel>

DWIDGET_USE_NAMESPACE

class locatorModel : public QAbstractListModel
{
public:
    enum Columns {
        DisplayNameColumn,
        ExtraInfoColumn,
        ColumnCount
    };

    locatorModel(QObject *parent = nullptr)
        : QAbstractListModel(parent) {}
    void clear();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayPropertyRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void addItems(const QList<baseLocatorItem> &items);

    QList<baseLocatorItem> getItems();

private:
    QList<baseLocatorItem> items;
};

class PopupWidget : public DFrame
{
    Q_OBJECT
public:
    PopupWidget(QWidget *parent = nullptr);
    void setmodel(locatorModel *model);
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
    DTreeView *tree { nullptr };
    locatorModel *model { nullptr };
};

#endif   // POPUPWIDGET_H
