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
#ifndef SESSIONDEBUG_H
#define SESSIONDEBUG_H

#include "session.h"
#include <QObject>

#include <memory>

class AbstractDebugger;
namespace dap {
    class Session;
}
class SessionDebug : public QObject
{
    Q_OBJECT
public:
    explicit SessionDebug(QObject *parent = nullptr);

    void setDebugger(AbstractDebugger *dbg);

    bool initialize();
    bool registerHandlers();

signals:

public slots:
    void onError(const char *err);

private:
    AbstractDebugger *debugger = nullptr;
    std::unique_ptr<dap::Session> session;
};

#endif // SESSIONDEBUG_H
