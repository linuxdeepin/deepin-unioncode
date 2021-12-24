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
#ifndef DAPDEBUGGER_H
#define DAPDEBUGGER_H

#include "abstractdebugger.h"


#include <QObject>


/**
 * @brief Abstraction of debugger, actual operation
 * processed by remote debugger through DAP protocol.
 */
class DapDebugger : public AbstractDebugger
{
    Q_OBJECT
public:
    explicit DapDebugger(QObject *parent = nullptr);

    void startDebug() override;
    void detachDebug() override;

    void interruptDebug() override;
    void continueDebug() override;
    void abortDebug() override;
    void restartDebug() override;

    void stepOver() override;
    void stepIn() override;
    void stepOut() override;
signals:

public slots:

};

#endif // DAPDEBUGGER_H
