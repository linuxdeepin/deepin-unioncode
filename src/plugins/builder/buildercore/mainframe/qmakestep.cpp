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
#include "qmakestep.h"

#include "QtConcurrent"

QMakeStep::QMakeStep(QObject *parent) : BuildStep(parent)
{
    // TODO(mozart) : those parameters should get from other place.
    qmakeArgs << "-spec" << "linux-g++" << "CONFIG+=debug" << "CONFIG+=qml_debug";
    qmakeCmd = "qmake";

    makeArgs << "-j4";
    makeCmd = "make";
}

bool QMakeStep::run()
{
    bool ret = runQMake() & runMake();
    return ret;
}

bool QMakeStep::runMake()
{
    return execCmd(makeCmd, makeArgs);
}

bool QMakeStep::runQMake()
{
    QStringList processArgs;
    processArgs << makeFile << qmakeArgs;
    return execCmd(qmakeCmd, processArgs);
}
