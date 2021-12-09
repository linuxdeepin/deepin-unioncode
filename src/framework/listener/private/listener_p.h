/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef LISTENER_P_H
#define LISTENER_P_H

#include "framework/framework_global.h"

#include <QObject>

DPF_BEGIN_NAMESPACE

class Listener;
class ListenerPrivate final : public QObject
{
    Q_OBJECT
    friend class Listener;
    Listener *const q;
    friend class PluginManagerPrivate;
public:
    explicit ListenerPrivate(Listener *parent);
signals:
    void pluginsInitialized();
    void pluginsStarted();
    void pluginsStoped();
};

DPF_END_NAMESPACE

#endif // LISTENER_P_H
