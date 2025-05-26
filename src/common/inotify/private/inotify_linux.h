// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INOTIFY_LINUX_H
#define INOTIFY_LINUX_H

#include "inotify_hook.h"

#include <QDebug>

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

class InotifyLinux : public InotifyHook
{
    Q_OBJECT
    int inotifyFD {0};
    struct pollfd pfdCache {};
    QHash<Type, int> typeMapping
    {
        { Type::ACCESS, IN_ACCESS },
        { Type::MODIFY, IN_MODIFY },
        { Type::ATTRIB, IN_ATTRIB },
        { Type::CLOSE_WRITE, IN_CLOSE_WRITE },
        { Type::CLOSE_NOWRITE, IN_CLOSE_NOWRITE },
        { Type::CLOSE, IN_CLOSE },
        { Type::OPEN, IN_OPEN },
        { Type::MOVED_FROM, IN_MOVED_FROM },
        { Type::MOVED_TO, IN_MOVED_TO },
        { Type::MOVE, IN_MOVE },
        { Type::CREATE, IN_CREATE },
        { Type::DELETE, IN_DELETE },
        { Type::DELETE_SELF, IN_DELETE_SELF },
        { Type::MOVE_SELF, IN_MOVE_SELF },
    };
    QHash<int, QString> watchPaths {};
    QReadWriteLock rwLock {};

public:
    InotifyLinux()
    {
        inotifyFD = inotify_init1(IN_NONBLOCK);
        if (inotifyFD == -1) {
            qCritical() << "Failed, create inotify fd";
        }
        pfdCache = {inotifyFD, POLLIN, 0};
    }

    ~InotifyLinux() override
    {
        if (inotifyFD > 0) {
            for(auto wd : watchPaths.keys()) {
                inotify_rm_watch(inotifyFD, wd);
            }
            close(inotifyFD);
        }
    }

    virtual void addPath(const QString &path) override
    {
        if (inotifyFD == -1)
            return;

        if (watchPaths.values().contains(path))
            removePath(path);

        QWriteLocker lock(&rwLock);
        int watcherID = inotify_add_watch(inotifyFD, path.toLatin1(),
                                          IN_MODIFY| IN_OPEN| IN_CLOSE| IN_CREATE|
                                          IN_MOVED_TO| IN_MOVED_FROM| IN_MOVE_SELF| IN_MOVE|
                                          IN_DELETE| IN_DELETE_SELF);

        if (watcherID == -1) {
            qCritical() << "Failed, Create watcher from called inotify_add_watch";
            return;
        }

        watchPaths[watcherID] = path;
    }

    virtual void removePath(const QString &path) override
    {
        QWriteLocker lock(&rwLock);

        int wd = watchPaths.key(path);
        if (wd < 0)
            return;

        inotify_rm_watch(inotifyFD, wd);
        watchPaths.remove(wd);
    }

    virtual void run() override
    {
        if (inotifyFD == -1) {
            return;
        }

        while(!stopFlag) {
            int poll_num = -1;
            poll_num = poll(&pfdCache, 1, 15);
            if (poll_num == -1) {
                if (errno == EINTR)
                    continue;
                qCritical() << "Failed, Create poll instance from called poll()";
                return;
            }
            if (poll_num > 0) {
                if (pfdCache.revents & POLLIN) {
                    char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
                    const struct inotify_event *event;
                    for (;;) {
                        ssize_t len = read(inotifyFD, buf, sizeof(buf));
                        if (len == -1 && errno != EAGAIN) {
                            continue;
                        }
                        if (len <= 0)
                            break;
                        for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
                            QReadLocker lock(&rwLock);

                            event = (const struct inotify_event *) ptr;

                            if (watchPaths.isEmpty())
                                continue;

                            QString filePath = watchPaths.value(event->wd);
                            if (filePath.isEmpty())
                                continue;

                            for (auto val : typeMapping.values()) {
                                if (event->mask & val) {
                                    auto type = typeMapping.key(val);
                                    emit inotifyEvent(type, filePath);
                                    break;
                                }
                            }
                        }
                    } // for read event
                }
            }
        }
    }
};

#endif // INOTIFY_LINUX_H
