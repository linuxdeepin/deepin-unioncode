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
#ifndef ABSTRACTDEBUGGER_H
#define ABSTRACTDEBUGGER_H

#include <QObject>
#include "debuggerrunparameters.h"

/**
 * @brief TODO(mozart):AbstractDebugger class just for promote development, may
 * be deleted future.
 */
class AbstractDebugger : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDebugger(QObject *parent = nullptr);
    virtual ~AbstractDebugger() {}

    virtual void setRunParameters(DebuggerRunParameters &_rp);
    virtual void startDebug() = 0;
    virtual void detachDebug() = 0;

    virtual void interruptDebug() = 0;
    virtual void continueDebug() = 0;
    virtual void abortDebug() = 0;
    virtual void restartDebug() = 0;

    virtual void stepOver() = 0;
    virtual void stepIn() = 0;
    virtual void stepOut() = 0;
signals:

public slots:

protected:
    DebuggerRunParameters rp;
};

#endif // ABSTRACTDEBUGGER_H
