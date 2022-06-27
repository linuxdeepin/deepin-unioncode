/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "commandstep.h"
#include "tasks/gnumakeparser.h"
#include "tasks/gccparser.h"

#include <QTimer>

CommandStep::CommandStep(QObject *parent) : BuildStep(parent)
{
    setOutputParser(new GnuMakeParser());
    appendOutputParser(new GccParser());
}

void CommandStep::setCommand(const QString &_cmd, const QStringList &_params)
{
    cmd = _cmd;
    cmdParams = _params;
}

bool CommandStep::run()
{
    QString printText;
    printText += cmd;
    printText += " ";
    printText += cmdParams.join(" ");

    emit addOutput("Start running: " + printText, OutputFormat::NormalMessage);
    bool success = runCommand();
    QTimer::singleShot(0, this, [this]{
        emit addOutput("Run finished", OutputFormat::NormalMessage);
        this->deleteLater();
    });
    return success;
}

bool CommandStep::runCommand()
{
    return execCmd(cmd, cmdParams);
}
