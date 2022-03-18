/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#ifndef WINDOWSWITCHER_H
#define WINDOWSWITCHER_H

#include "windowkeeper.h"

#include <framework/framework.h>

#include <QMainWindow>

//框架实例化
class WindowSwitcher : public dpf::EventHandler, dpf::AutoEventHandlerRegister<WindowSwitcher>
{
    Q_OBJECT
public:
    explicit WindowSwitcher(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event& event);
    virtual void navEvent(const dpf::Event& event);
};
#endif // WINDOWSWITCHER_H
