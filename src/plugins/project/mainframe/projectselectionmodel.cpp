// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectselectionmodel.h"

ProjectSelectionModel::ProjectSelectionModel(QAbstractItemModel *model)
    : QItemSelectionModel (model)
{

}

ProjectSelectionModel::ProjectSelectionModel(QAbstractItemModel *model, QObject *parent)
    : QItemSelectionModel (model, parent)
{

}
