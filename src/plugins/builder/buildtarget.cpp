/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#include "buildtarget.h"

#include "builderreceiver.h"
#include "buildstep.h"
#include "project.h"
#include "qmakestep.h"
#include "makestep.h"
#include "builderreceiver.h"


BuildTarget::BuildTarget(Project *pro)
    : project(pro)
{
    assert(project);
}

BuildTarget::~BuildTarget()
{
    clear();
}

QString BuildTarget::buildOutputDirectory() const
{
    const QString &buildOutputPath = BuilderReceiver::instance()->buildOutputDirectory();
    QDir dir(buildOutputPath);
    if (!dir.exists()) {
        dir.mkpath(buildOutputPath);
    }
    return buildOutputPath;
}

QList<BuildStep *> &BuildTarget::getbuildSteps()
{
    clear();
    constructBuildSteps();
    return buildSteps;
}

bool BuildTarget::constructBuildSteps()
{
    bool ret = true;
    BuildStep *step = nullptr;
    if (project) {
        ToolChainType tlChainType = project->toolChainType();
        switch (tlChainType) {
        case ToolChainType::QMake:
            step = new QMakeStep;
            break;
        case ToolChainType::CMake:
            step = new MakeStep;
            break;
        default:
            ret = false;
        }

        if (step) {
            step->setBuildOutputDir(buildOutputDirectory());
            step->appendCmdParam("-B"); // TODO(mozart) : those params not used and should get from other place.
            step->appendCmdParam("-j4");
            step->setMakeFile(project->projectFilePath());
            buildSteps << step;
        }
    }

    return ret;
}

void BuildTarget::clear()
{
    for (auto it : buildSteps) {
        if (it) {
            delete it;
            it = nullptr;
        }
    }
    buildSteps.clear();
}


