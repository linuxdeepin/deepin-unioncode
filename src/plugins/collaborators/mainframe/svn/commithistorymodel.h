/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef COMMITHISTORYMODEL_H
#define COMMITHISTORYMODEL_H

#include "commitdatarole.h"

#include <QAbstractItemModel>

namespace collaborators {

class CommitHistoryModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit CommitHistoryModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int s, Qt::Orientation o, int role = Qt::DisplayRole) const override;
    QModelIndex index(int r, int c, const QModelIndex &par = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &par = QModelIndex()) const override;
    bool hasChildren(const QModelIndex &par = QModelIndex()) const override;
    int columnCount(const QModelIndex &) const override;

private:
    QHash<CommitDataRole, QString> mColumns;
};

} //namespace collaborators

#endif // COMMITHISTORYMODEL_H
