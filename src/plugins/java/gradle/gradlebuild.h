// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRADLEBUILD_H
#define GRADLEBUILD_H

#include "dap/protocol.h"
#include <QObject>

class GradleBuildPrivate;
class GradleBuild : public QObject
{
    Q_OBJECT
public:
    explicit GradleBuild(QObject *parent = nullptr);
    ~GradleBuild();

    static QString build(const QString& kitName, const QString& projectPath);

signals:

private slots:

private:

private:
    GradleBuildPrivate *const d;
};

#endif //GRADLEBUILD_H

