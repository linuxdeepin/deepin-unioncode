// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESOURCEMODEL_H
#define FILESOURCEMODEL_H

#include <QFileSystemModel>

class FileSourceModel : public QFileSystemModel
{
    Q_OBJECT
public:
    explicit FileSourceModel(QObject *parent = nullptr);
};

#endif // FILESOURCEMODEL_H
