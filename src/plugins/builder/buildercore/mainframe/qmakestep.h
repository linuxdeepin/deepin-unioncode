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
#ifndef QMAKESTEP_H
#define QMAKESTEP_H

#include "buildstep.h"

class QMakeStep : public BuildStep
{
    Q_OBJECT
public:
    explicit QMakeStep(QObject *parent = nullptr);

    virtual bool run() override;

signals:

private:
    bool runMake();
    bool runQMake();

    QStringList qmakeArgs;
    QString qmakeCmd;

    QStringList makeArgs;
    QString makeCmd;
};

#endif // QMAKESTEP_H
