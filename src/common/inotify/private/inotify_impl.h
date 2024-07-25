// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INOTIFY_IMPL_H
#define INOTIFY_IMPL_H

#include "inotify_hook.h"
#include "inotify_linux.h"
#include <QDebug>
#include <QObject>
#include <QThread>

class InotifyImpl : public QObject
{
    Q_OBJECT
public:
    static InotifyHook *createInotify()
    {
        InotifyHook *inotifyHook = nullptr;
#ifdef __linux__
        inotifyHook = new InotifyLinux();
#endif
        if (!inotifyHook) {
            qCritical() << "Failed, create inotify hook!";
        }
        inotifyHook->start();
        inotifyHook->metaObject()->invokeMethod(nullptr, "start");
        return inotifyHook;
    }

    static void removeInotify(InotifyHook *hook)
    {
        if (hook)
            hook->stop();
        while (!hook->isFinished());
        delete hook;
    }
};

#endif // INOTIFY_IMPL_H
