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
#ifndef MAKESTEP_H
#define MAKESTEP_H

#include <buildstep.h>
#include <QObject>

class QProcess;
class MakeStep : public BuildStep
{
    Q_OBJECT
public:
    explicit MakeStep(QObject *parent = nullptr);

    virtual void run() override;
signals:


private:
    bool runMake();
    bool runCMake();

    QStringList cmakeArgs;
    QString cmakeCmd;

    QStringList makeArgs;
    QString makeCmd;
};

#endif // MAKESTEP_H
