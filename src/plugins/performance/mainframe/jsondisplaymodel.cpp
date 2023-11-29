// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jsondisplaymodel.h"

namespace  {
static std::string feild{"feild"};
static std::string lines{"lines"};
}

class JsonDisplayModelPrivate
{
    friend class JsonDisplayModel;
    Json::Value cacheValue;
    QStringList cacheFeild;
    QVector<QStringList> cacheLines;
};

JsonDisplayModel::JsonDisplayModel(QObject *parent)
    : QStandardItemModel (parent)
    , d (new JsonDisplayModelPrivate)
{

}

JsonDisplayModel::~JsonDisplayModel()
{
    if (d)
        delete d;
}

void JsonDisplayModel::parseJson(const Json::Value &value)
{
    d->cacheValue = value;
    Json::Value feild = d->cacheValue[::feild];
    Json::Value lines = d->cacheValue[::lines];
    if (d->cacheFeild.isEmpty()) {
        if (!feild.empty()) {
            for (uint idx = 0; idx < feild.size(); idx ++) {
                d->cacheFeild << QString::fromStdString(feild[idx].asString());
            }
            setColumnCount(d->cacheFeild.size());
        }
    }

    if (!lines.empty()) {
        setRowCount(0);
        d->cacheLines.clear();
        for (auto line : lines) {
            QStringList lineData;
            for (auto feildCacheOne : d->cacheFeild) {
                int findIdx = -1;
                for (uint idx = 0; idx < feild.size(); idx ++) {
                    if (feildCacheOne.toStdString() == feild[idx].asString()) {
                        findIdx = idx;
                        break;
                    }
                }
                if (findIdx >= 0) {
                    lineData << QString::fromStdString(line[findIdx].asString());
                }
            }
            d->cacheLines << lineData;
        }
        setRowCount(d->cacheLines.size());
    }
}

QVariant JsonDisplayModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
    if (d->cacheFeild.isEmpty() || d->cacheValue[::feild].empty())
        return {};

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal
            && section >= 0 && section < d->cacheFeild.size()) {
        QString feildOne = d->cacheFeild[section];
        return feildOne;
    }
    return {};
}

bool JsonDisplayModel::setHeaderData(int section, Qt::Orientation orientation,
                                     const QVariant &value, int role)
{
    if (d->cacheFeild.empty()) {
        return QStandardItemModel::setHeaderData(section, orientation, value, role);
    } else {
        return true;
    }
}

QModelIndex JsonDisplayModel::index(int row, int column, const QModelIndex &parent) const
{
    return QStandardItemModel::index(row, column, parent);
}

QVariant JsonDisplayModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    int row = index.row();
    int col = index.column();

    if (row >= 0  && col >= 0) {
        if (role == Qt::DisplayRole) {
            return d->cacheLines[row][col];
        }
    }
    return QStandardItemModel::data(index, role);
}
