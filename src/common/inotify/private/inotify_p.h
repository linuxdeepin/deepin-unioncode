// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INOTIFY_P_H
#define INOTIFY_P_H

#include "inotify_impl.h"

#include <QObject>
class Inotify;
class InotifyPrivate : public QObject
{
    friend class Inotify;
    Inotify *const q;
public:
    InotifyPrivate(Inotify *q);
    virtual ~InotifyPrivate();
    void addPath(const QString &path);
    void removePath(const QString &path);

private slots:
    void doInotifyEvent(InotifyHook::Type type, const QString &path);

private:
    QList<QString> ignoreList;
    InotifyHook* hook{nullptr};
};
#endif // INOTIFY_P_H
