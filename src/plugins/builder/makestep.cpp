/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "makestep.h"
#include "tasks/gnumakeparser.h"
#include "tasks/gccparser.h"

#include <QTimer>

MakeStep::MakeStep(QObject *parent) : BuildStep(parent)
{
    // TODO(mozart) : those parameters should get from other place.
    cmakeArgs <<"-DCMAKE_BUILD_TYPE=Debug" << "-B";
    cmakeCmd = "cmake";

    makeArgs << "-j4";
    makeCmd = "make";

    setOutputParser(new GnuMakeParser());
    appendOutputParser(new GccParser());
}

void MakeStep::run()
{
    emit addOutput("Build starts", OutputFormat::NormalMessage);
    runCMake();
    runMake();
    QTimer::singleShot(0, this, [this]{
        emit addOutput("Build has finished", OutputFormat::NormalMessage);
    });
}

bool MakeStep::runMake()
{
    return execCmd(makeCmd, makeArgs);
}

bool MakeStep::runCMake()
{
    QStringList processArgs;
    processArgs << makeFile << cmakeArgs << buildOutputDir;
    return execCmd(cmakeCmd, processArgs);
}
