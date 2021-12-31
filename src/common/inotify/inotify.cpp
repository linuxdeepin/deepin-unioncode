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
#include "inotify.h"

#include <QHash>
#include <QRunnable>
#include <QtConcurrent>

#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

static QHash<Inotify::Type, int> typeMapping
{
    { Inotify::Type::ACCESS, IN_ACCESS },
    { Inotify::Type::MODIFY, IN_MODIFY },
    { Inotify::Type::ATTRIB, IN_ATTRIB },
    { Inotify::Type::CLOSE_WRITE, IN_CLOSE_WRITE },
    { Inotify::Type::CLOSE_NOWRITE, IN_CLOSE_NOWRITE },
    { Inotify::Type::CLOSE, IN_CLOSE },
    { Inotify::Type::OPEN, IN_OPEN },
    { Inotify::Type::MOVED_FROM, IN_MOVED_FROM },
    { Inotify::Type::MOVED_TO, IN_MOVED_TO },
    { Inotify::Type::MOVE, IN_MOVE },
    { Inotify::Type::CREATE, IN_CREATE },
    { Inotify::Type::DELETE, IN_DELETE },
    { Inotify::Type::DELETE_SELF, IN_DELETE_SELF },
    { Inotify::Type::MOVE_SELF, IN_MOVE_SELF },
};

typedef QHash<int, QString> WatcherHash;

class PollRunable : public QRunnable
{
    typedef std::function<void(const QString &filePath)> FuncCB;
    int inotifyFd = -1;
    WatcherHash *watcherHash;
    pollfd *pfdCache = nullptr;
    nfds_t numCache = 0;
    int timeOut = -1;
    QHash<Inotify::Type, FuncCB> callbacks;
    bool stopFlag = false;
public:
    PollRunable(int inotifyFd, WatcherHash* hash, pollfd *pfd, nfds_t num  = 1 ,int timeout = -1) :
        inotifyFd(inotifyFd), watcherHash(hash), pfdCache(pfd), numCache(num), timeOut(timeout) { }
    void setWatcherCallback(Inotify::Type type,FuncCB cb) { callbacks[type] = cb;}
    void stop() { stopFlag = true; }
    virtual void run() override;
};

class InotifyPrivate
{
    friend class Inotify;
    int inotifyFD = -1;
    struct pollfd inotifyInput{};
    WatcherHash watchers{};
    PollRunable *runable = nullptr;
};

Inotify::Inotify(QObject *parent)
    : QObject(parent)
    , d(new InotifyPrivate())
{
    d->inotifyFD = inotify_init1(IN_NONBLOCK);
    if (d->inotifyFD == -1) {
        qCritical() << "Failed, Create inotify from called inotify_init1";
        abort();
    }

    d->inotifyInput.fd = d->inotifyFD;
    d->inotifyInput.events = POLLIN;

    d->runable = new PollRunable(d->inotifyFD, &d->watchers, &(d->inotifyInput));
    d->runable->setWatcherCallback(MODIFY, [=](const QString &filePath){this->modified(filePath);});
    d->runable->setWatcherCallback(CLOSE, [=](const QString &filePath){this->closed(filePath);});
    d->runable->setWatcherCallback(OPEN, [=](const QString &filePath){this->opened(filePath);});
    d->runable->setWatcherCallback(MOVE, [=](const QString &filePath){this->movedSub(filePath);});
    d->runable->setWatcherCallback(DELETE, [=](const QString &filePath){this->deletedSub(filePath);});
    d->runable->setWatcherCallback(CREATE, [=](const QString &filePath){this->createdSub(filePath);});
    d->runable->setWatcherCallback(DELETE_SELF, [=](const QString &filePath){this->deletedSelf(filePath);});
    d->runable->setWatcherCallback(MOVE_SELF, [=](const QString &filePath){this->movedSelf(filePath);});
    QThreadPool::globalInstance()->start(d->runable);
}

Inotify::~Inotify()
{
    if (d) {
        if (d->runable) {
            d->runable->stop();
            QThreadPool::globalInstance()->waitForDone();
            delete d->runable;
        }
        delete d;
    }
}

Inotify *Inotify::globalInstance()
{
    static Inotify ins;
    return &ins;
}

bool Inotify::addPath(const QString &path)
{
    if (d->inotifyFD == -1)
        return false;

    int watcherID = inotify_add_watch(d->inotifyFD, path.toLatin1(),
                                      IN_MODIFY| IN_OPEN| IN_CLOSE| IN_CREATE|
                                      IN_MOVED_TO| IN_MOVED_FROM| IN_MOVE_SELF| IN_MOVE|
                                      IN_DELETE| IN_DELETE_SELF);

    if (watcherID == -1) {
        qInfo() << "Failed, Create watcher from called inotify_add_watch";
        return false;
    }

    d->watchers[watcherID] = path;
    return true;
}

void Inotify::removePath(const QString &path)
{
    int wd = d->watchers.key(path);
    if (wd < 0)
        return;

    inotify_rm_watch(d->inotifyFD, wd);
    d->watchers.remove(wd);
}

void PollRunable::run()
{
    while(!stopFlag && pfdCache && inotifyFd > 0) {
        int poll_num = -1;
        poll_num = poll(pfdCache, numCache, timeOut);
        if (poll_num == -1) {
            if (errno == EINTR)
                continue;
            qCritical() << "Failed, Create poll instance from called poll()";
            abort();
        }
        if (poll_num > 0) {
            if (pfdCache->revents & POLLIN) {
                char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
                const struct inotify_event *event;
                for (;;) {
                    ssize_t len = read(inotifyFd, buf, sizeof(buf));
                    if (len == -1 && errno != EAGAIN) {
                        qCritical() << "Failed, poll read events";
                        abort();
                    }
                    if (len <= 0)
                        break;
                    for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
                        event = (const struct inotify_event *) ptr;
                        QString filePath = watcherHash->value(event->wd);
                        if (!filePath.isEmpty()) {
                            if (event->mask & IN_OPEN) {
                                FuncCB callback = callbacks[typeMapping.key(IN_OPEN)];
                                if (callback) {
                                    callback(filePath);
                                }
                            }
                            if (event->mask & IN_CLOSE) {
                                FuncCB callback = callbacks[typeMapping.key(IN_CLOSE)];
                                if (callback) {
                                    callback(filePath);
                                }
                            }
                            if (event->mask & IN_CREATE) {
                                FuncCB callback = callbacks[typeMapping.key(IN_CREATE)];
                                if (callback) {
                                    callback(filePath);
                                }
                            }
                            if (event->mask & IN_DELETE) {
                                FuncCB callback = callbacks[typeMapping.key(IN_DELETE)];
                                if (callback) {
                                    callback(filePath);
                                }
                            }
                            if (event->mask & IN_MOVE) {
                                FuncCB callback = callbacks[typeMapping.key(IN_MOVE)];
                                if (callback) {
                                    callback(filePath);
                                }
                            }
                            if (event->mask & IN_MODIFY) {
                                FuncCB callback = callbacks[typeMapping.key(IN_MODIFY)];
                                if (callback) {
                                    callback(filePath);
                                }
                            }
                            if (event->mask & IN_DELETE_SELF) {
                                FuncCB callback = callbacks[typeMapping.key(IN_DELETE_SELF)];
                                if (callback) {
                                    callback(filePath);
                                }
                            }
                        }
                        /* Print type of filesystem object. */
                        // if (event->mask & IN_ISDIR)
                        // printf(" [directory]\n");
                        // else
                        // printf(" [file]\n");
                    }
                }
            }
        }
    }
}
