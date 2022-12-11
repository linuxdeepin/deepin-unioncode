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
#ifndef NINJAGENERATOR_H
#define NINJAGENERATOR_H

#include "services/builder/buildergenerator.h"

class NinjaGeneratorPrivate;
class NinjaGenerator : public dpfservice::BuilderGenerator
{
    Q_OBJECT
public:
    explicit NinjaGenerator();
    virtual ~NinjaGenerator() override;

    static QString toolKitName() { return "ninja"; }

    BuildCommandInfo getMenuCommand(const BuildMenuType buildMenuType, const dpfservice::ProjectInfo &projectInfo) override;
    void appendOutputParser(std::unique_ptr<IOutputParser> &outputParser) override;
    bool checkCommandValidity(const BuildCommandInfo &info, QString &retMsg) override;

signals:

private slots:
private:
    NinjaGeneratorPrivate *const d;
};

#endif // NINJAGENERATOR_H
