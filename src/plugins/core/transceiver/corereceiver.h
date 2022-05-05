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
#ifndef CORERECEIVER_H
#define CORERECEIVER_H

#include <framework/framework.h>

#include <QMainWindow>

//框架实例化
class CoreReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<CoreReceiver>
{
    Q_OBJECT
public:
    explicit CoreReceiver(QObject *parent = nullptr);
    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event& event);
    virtual void navEvent(const dpf::Event& event);
    virtual void processMessageEvent(const dpf::Event &event);
};
#endif // CORERECEIVER_H
