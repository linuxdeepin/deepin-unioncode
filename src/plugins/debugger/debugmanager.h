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
#ifndef DEBUGMANAGER_H
#define DEBUGMANAGER_H

#include "abstractdebugger.h"
#include "sessiondebug.h"

#include <QObject>

class DebugManager : public AbstractDebugger
{
    Q_OBJECT
public:
    explicit DebugManager(QObject *parent = nullptr);
    bool initialize();

signals:

public slots:
    void startDebug() override;
    void detachDebug() override;

    void interruptDebug() override;
    void continueDebug() override;
    void abortDebug() override;
    void restartDebug() override;

    void stepOver() override;
    void stepIn() override;
    void stepOut() override;

private:
    QScopedPointer<AbstractDebugger> debugger;
    QScopedPointer<SessionDebug> sessionDebug;
};

#endif // DEBUGMANAGER_H
