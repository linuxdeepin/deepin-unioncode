// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTSELECTIONMODEL_H
#define PROJECTSELECTIONMODEL_H

#include <QItemSelectionModel>

class ProjectSelectionModel : public QItemSelectionModel
{
public:
    explicit ProjectSelectionModel(QAbstractItemModel *model = nullptr);
    explicit ProjectSelectionModel(QAbstractItemModel *model, QObject *parent);
};

#endif // PROJECTSELECTIONMODEL_H
