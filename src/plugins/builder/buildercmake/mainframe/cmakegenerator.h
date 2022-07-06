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
#ifndef CMAKEGENERATOR_H
#define CMAKEGENERATOR_H

#include "services/builder/buildergenerator.h"

#include <QObject>

class CMakeGeneratorPrivate;
class CMakeGenerator : public dpfservice::BuilderGenerator
{
    Q_OBJECT
    CMakeGeneratorPrivate *const d;
public:
    explicit CMakeGenerator();
    virtual ~CMakeGenerator() override;

    static QString toolKitName() { return "cmake"; }

signals:

private slots:

private:
};

#endif // CMAKEGENERATOR_H
