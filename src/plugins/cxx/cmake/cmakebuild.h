// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKEBUILD_H
#define CMAKEBUILD_H

#include "dap/protocol.h"
#include <QObject>

class CMakeBuildPrivate;
class CMakeBuild : public QObject
{
    Q_OBJECT
public:
    explicit CMakeBuild(QObject *parent = nullptr);
    ~CMakeBuild();

    static QString build(const QString& kitName, const QString& projectPath);

signals:

private slots:

private:

private:
    CMakeBuildPrivate *const d;
};

#endif //CMAKEBUILD_H

