// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKEITEMMODEL_H
#define CMAKEITEMMODEL_H

#include <QAbstractItemModel>

class CMakeItem
{
public:
    //STATIC/INTERNAL means this variable is internal used in Cmake, do not operate it directly
    enum itemType {
        FILEPATH,
        PATH,
        BOOL,
        STRING,
        INTERNAL,
        STATIC
    };

    CMakeItem();
    CMakeItem(const QByteArray *key, const QByteArray *value);
    static itemType dataToType(const QByteArray &data);
    static QString typeToString(itemType type);

    QString key;
    QVariant value;
    QString description;
    itemType type = STRING;
    bool isAdvanced = false;
    bool isUnset = false;
    bool isInCacheFile = false;
    bool isChanged = false;
};

class CMakeItemModel : public QAbstractTableModel
{
public:
    enum columnType {
        Key,
        Value
    };

    explicit CMakeItemModel(QObject *parent = nullptr);
    ~CMakeItemModel() override;

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    CMakeItem itemOfIndex(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    void setData(const QList<CMakeItem> &data);

    QList<CMakeItem> getItems();
private:
    QList<CMakeItem> cmakeData;
    bool showAdvanced { false };
};

#endif // CMAKEITEMMODEL_H
