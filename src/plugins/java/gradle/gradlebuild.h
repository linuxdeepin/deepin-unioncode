/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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

