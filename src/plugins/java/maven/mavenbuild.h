// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAVENBUILD_H
#define MAVENBUILD_H

#include "dap/protocol.h"
#include <QObject>

class MavenBuildPrivate;
class MavenBuild : public QObject
{
    Q_OBJECT
public:
    explicit MavenBuild(QObject *parent = nullptr);
    ~MavenBuild();

    static QString build(const QString& kitName, const QString& projectPath);

signals:

private slots:

private:

private:
    MavenBuildPrivate *const d;
};

#endif //MAVENBUILD_H

