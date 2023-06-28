// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYMBOLMODEL_H
#define SYMBOLMODEL_H

#include <QFileSystemModel>

class SymbolModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit SymbolModel(QObject *parent = nullptr);

protected:
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
};

#endif // SYMBOLMODEL_H
