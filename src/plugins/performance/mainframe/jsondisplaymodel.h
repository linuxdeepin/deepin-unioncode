// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JSONDISPLAYMODEL_H
#define JSONDISPLAYMODEL_H

#include <json/json.h>

#include <QStandardItemModel>

class JsonDisplayModelPrivate;
class JsonDisplayModel : public QStandardItemModel
{
    Q_OBJECT
    JsonDisplayModelPrivate *const d;
public:
    explicit JsonDisplayModel(QObject *parent = nullptr);
    virtual ~JsonDisplayModel();
public slots:
    void parseJson(const Json::Value &value);

protected:
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
                       int role = Qt::EditRole) override;
    virtual QModelIndex index(int row, int column,
                              const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

#endif // JSONDISPLAYMODEL_H
