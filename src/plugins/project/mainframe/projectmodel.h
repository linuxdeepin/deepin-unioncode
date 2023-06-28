// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include <QStandardItem>
class ProjectModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit ProjectModel(QObject *parent = nullptr);
};

#endif // PROJECTMODEL_H
