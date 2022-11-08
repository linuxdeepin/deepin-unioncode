/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "symbolmodel.h"
#include "definitions.h"

#include <QDebug>

SymbolModel::SymbolModel(QObject *parent)
    : QFileSystemModel (parent)
{
    setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
}

QVariant SymbolModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::ItemDataRole::DecorationRole) {
        return QVariant();
    } else if (role == Qt::ItemDataRole::ToolTipRole) {
        QFile file(filePath(index) + QDir::separator() + SymbolPri::recordFileName);
        if (file.exists()) {
            if (!file.open(QFile::ReadOnly)) {
                qCritical() << file.errorString();
            }
            QString data = file.readAll();
            data = data.remove(data.length() -1, 1);
            file.close();
            return data;
        }
    } else {
        // TODO:Huang yu
    }
    return QFileSystemModel::data(index, role);
}

int SymbolModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

bool SymbolModel::hasChildren(const QModelIndex &parent) const
{
    auto folder = fileInfo(parent).filePath();
    QDir dir(folder);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::Name);
    QStringList allFolder = dir.entryList();
    return !allFolder.isEmpty();
}
