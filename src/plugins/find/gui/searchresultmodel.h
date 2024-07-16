// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESULTMODEL_H
#define SEARCHRESULTMODEL_H

#include "constants.h"

#include <QAbstractItemModel>

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
    void setReplaceText(const QString &text, bool regex);
    FindItem *findItem(const QModelIndex &index) const;
    QString findGroup(const QModelIndex &index) const;
    void appendResult(const FindItemList &list);
    QMap<QString, FindItemList> allResult() const;
    QMap<QString, FindItemList> findResult(const QModelIndex &index) const;
    void remove(const QModelIndex &index);

Q_SIGNALS:
    void requestReplace(const QModelIndex &index);

private:
    void addGroup(const QString &group);
    void addItem(const QString &group, const FindItemList &itemList);
    void removeGroup(const QString &group);
    void removeItem(const QString &group, const FindItem &item);
    QVariant data(const FindItem &item, int role = Qt::DisplayRole) const;
    QVariant data(const QString &group, int role = Qt::DisplayRole) const;

    QMap<QString, FindItemList> resultData;
    QString replaceText;
    bool enableRegex { false };
};

#endif   // SEARCHRESULTMODEL_H
