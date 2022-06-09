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
#include "project.h"
#include "buildtarget.h"

#include "eventreceiver.h"

Project::Project(QObject *parent) : QObject(parent)
{
    curTarget = new BuildTarget(this);
}

QString Project::projectFilePath() const
{
    return EventReceiver::instance()->projectFilePath();
}

QString Project::projectDirectory() const
{
    return EventReceiver::instance()->projectDirectory();
}

QString Project::rootProjectDirectory() const
{
    return EventReceiver::instance()->rootProjectDirectory();
}

BuildTarget *Project::activeTarget() const
{
    return curTarget;
}

ToolChainType Project::toolChainType() const
{
    if (EventReceiver::instance()) {
        return EventReceiver::instance()->toolChainType();
    }
    return UnKnown;
}
