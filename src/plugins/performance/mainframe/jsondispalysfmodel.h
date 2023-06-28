// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JSONDISPALYSFMODEL_H
#define JSONDISPALYSFMODEL_H

#include <QSortFilterProxyModel>

class JsonDispalySFModelPrivate;
class JsonDispalySFModel : public QSortFilterProxyModel
{
    Q_OBJECT
    JsonDispalySFModelPrivate *const d;
public:
    explicit JsonDispalySFModel(QObject *parent = nullptr);
    virtual ~JsonDispalySFModel();
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
};

#endif // JSONDISPALYSFMODEL_H
