// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESULTTREEVIEW_H
#define SEARCHRESULTTREEVIEW_H

#include "constants.h"

#include <DTreeView>

class SearchResultModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SearchResultModel(QObject *parent = nullptr);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void clear();
    FindItem *findItem(const QModelIndex &index) const;
    QString findGroup(const QModelIndex &index) const;
    void appendResult(const FindItemList &list);
    QStringList fileList() const;

private:
    void addGroup(const QString &group);
    void addItem(const QString &group, const FindItemList &itemList);
    QVariant data(const FindItem &item, int role = Qt::DisplayRole) const;
    QVariant data(const QString &group, int role = Qt::DisplayRole) const;

    QMap<QString, FindItemList> resultData;
};

class SearchResultTreeViewPrivate;
class SearchResultTreeView : public DTK_WIDGET_NAMESPACE::DTreeView
{
    Q_OBJECT
public:
    explicit SearchResultTreeView(QWidget *parent = nullptr);
    ~SearchResultTreeView();

    void appendData(const FindItemList &itemList);
    void clearData();
    QStringList resultFileList() const;

private:
    SearchResultTreeViewPrivate *const d;
};

#endif   // SEARCHRESULTTREEVIEW_H
