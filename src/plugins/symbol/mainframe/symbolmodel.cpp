// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
