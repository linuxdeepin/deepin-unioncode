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
#include "gradlegenerator.h"
#include "parser/gradleparser.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

using namespace dpfservice;

class GradleGeneratorPrivate
{
    friend class GradleGenerator;
};

GradleGenerator::GradleGenerator()
    : d(new GradleGeneratorPrivate())
{

}

GradleGenerator::~GradleGenerator()
{
    if (d)
        delete d;
}

void GradleGenerator::getMenuCommand(BuildCommandInfo &info, const BuildMenuType buildMenuType)
{
    info.program = OptionManager::getInstance()->getGradleToolPath();
    switch (buildMenuType) {
    case Build:
        info.arguments.append("build");
        break;
    case Clean:
        info.arguments.append("clean");
        break;
    }
}

void GradleGenerator::appendOutputParser(std::unique_ptr<IOutputParser> &outputParser)
{
    if (outputParser) {
        outputParser->takeOutputParserChain();
        outputParser->appendOutputParser(new GradleParser());
    }
}


