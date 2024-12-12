// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "projectmodel.h"

#include "common/util/customicons.h"
#include "services/project/projectservice.h"

#include <QDebug>

ProjectModel::ProjectModel(QObject *parent)
    : QStandardItemModel(parent)
{
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DecorationRole) {
        auto name = index.data(Project::IconNameRole).toString();
        if (!name.isEmpty())
            return QIcon::fromTheme(name);

        name = index.data(Project::FileIconRole).toString();
        if (!name.isEmpty())
            return CustomIcons::icon(QFileInfo(name));
    }

    return QStandardItemModel::data(index, role);
}
