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
#ifndef GRADLEGENERATOR_H
#define GRADLEGENERATOR_H

#include "services/builder/buildergenerator.h"

class GradleGeneratorPrivate;
class GradleGenerator : public dpfservice::BuilderGenerator
{
    Q_OBJECT
public:
    explicit GradleGenerator();
    virtual ~GradleGenerator() override;

    static QString toolKitName() { return "gradle"; }

    void getMenuCommand(BuildCommandInfo &info, const BuildMenuType buildMenuType) override;
    void appendOutputParser(std::unique_ptr<IOutputParser> &outputParser) override;

signals:

private slots:
private:
    GradleGeneratorPrivate *const d;
};

#endif // GRADLEGENERATOR_H
