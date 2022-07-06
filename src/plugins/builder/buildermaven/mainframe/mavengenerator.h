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
#ifndef MAVENGENERATOR_H
#define MAVENGENERATOR_H

#include "services/builder/buildergenerator.h"

#include <QObject>

class MavenGeneratorPrivate;
class MavenGenerator : public dpfservice::BuilderGenerator
{
    Q_OBJECT
    MavenGeneratorPrivate *const d;
public:
    explicit MavenGenerator();
    virtual ~MavenGenerator() override;

    static QString toolKitName() { return "maven"; }

signals:

private slots:

private:
};

#endif // MAVENGENERATOR_H
