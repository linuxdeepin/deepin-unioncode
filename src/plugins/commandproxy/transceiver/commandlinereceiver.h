/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<huangyub@uniontech.com>
 *
 * Maintainer: hongjinchuan<huangyub@uniontech.com>
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
#ifndef COMMANDLINERECEIVER_H
#define COMMANDLINERECEIVER_H

#include "services/project/projectinfo.h"
#include "services/builder/builderglobals.h"

#include <framework/framework.h>

class CommandLineReceiver : public dpf::EventHandler,
        dpf::AutoEventHandlerRegister<CommandLineReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<CommandLineReceiver>;

public:
    explicit CommandLineReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

#endif // COMMANDLINERECEIVER_H
