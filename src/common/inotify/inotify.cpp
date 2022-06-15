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
#include <QReadWriteLock>

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
typedef QList<QString> IgnoreList;

class PollRunable : public QRunnable
{
    friend class Inotify;
    typedef std::function<void(const QString &filePath)> FuncCB;
    int inotifyFd = -1;
    WatcherHash *watcherHash = nullptr;
    IgnoreList *ignores = nullptr;
    pollfd *pfdCache = nullptr;
    nfds_t numCache = 0;
    int timeOut = -1;
    QHash<Inotify::Type, FuncCB> callbacks{};
    QHash<Inotify::Type, FuncCB> ignoreCallbacks{};
    bool stopFlag = false;
public:
    PollRunable(int inotifyFd, WatcherHash* hash, IgnoreList *list, pollfd *pfd, nfds_t num  = 1 ,int timeout = -1)
        : inotifyFd(inotifyFd), watcherHash(hash), ignores(list), pfdCache(pfd), numCache(num), timeOut(timeout) { }
    virtual ~PollRunable(){ qInfo() << "class PollRunable destoryed"; }
    void setWatcherCallback(Inotify::Type type, FuncCB cb) { callbacks[type] = cb;}
    void setWatcherIgnoreCallback(Inotify::Type type, FuncCB cb) {ignoreCallbacks[type] = cb;}
    void stop() { stopFlag = true; }
    virtual void run() override;
};

class InotifyPrivate
{
    friend class Inotify;
    int inotifyFD = -1;
    struct pollfd inotifyInput{};
    WatcherHash watchers{};
    IgnoreList ignores{};
    PollRunable *runable = nullptr;
};

namespace  {
static QReadWriteLock rwLock;
}

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

    d->runable = new PollRunable(d->inotifyFD, &d->watchers, &d->ignores, &(d->inotifyInput), 1, 25);
    d->runable->setAutoDelete(false);
    d->runable->setWatcherCallback(MODIFY, [=](const QString &filePath){this->modified(filePath);});
    d->runable->setWatcherCallback(CLOSE, [=](const QString &filePath){this->closed(filePath);});
    d->runable->setWatcherCallback(OPEN, [=](const QString &filePath){this->opened(filePath);});
    d->runable->setWatcherCallback(MOVE, [=](const QString &filePath){this->movedSub(filePath);});
    d->runable->setWatcherCallback(DELETE, [=](const QString &filePath){this->deletedSub(filePath);});
    d->runable->setWatcherCallback(CREATE, [=](const QString &filePath){this->createdSub(filePath);});
    d->runable->setWatcherCallback(DELETE_SELF, [=](const QString &filePath){this->deletedSelf(filePath);});
    d->runable->setWatcherCallback(MOVE_SELF, [=](const QString &filePath){this->movedSelf(filePath);});

    d->runable->setWatcherIgnoreCallback(MODIFY, [=](const QString &filePath){this->ignoreModified(filePath);});
    d->runable->setWatcherIgnoreCallback(CLOSE, [=](const QString &filePath){this->ignoreClosed(filePath);});
    d->runable->setWatcherIgnoreCallback(OPEN, [=](const QString &filePath){this->ignoreOpened(filePath);});
    d->runable->setWatcherIgnoreCallback(MOVE, [=](const QString &filePath){this->ignoreMovedSub(filePath);});
    d->runable->setWatcherIgnoreCallback(DELETE, [=](const QString &filePath){this->ignoreDeletedSub(filePath);});
    d->runable->setWatcherIgnoreCallback(CREATE, [=](const QString &filePath){this->ignoreCreatedSub(filePath);});
    d->runable->setWatcherIgnoreCallback(DELETE_SELF, [=](const QString &filePath){this->ignoreDeletedSelf(filePath);});
    d->runable->setWatcherIgnoreCallback(MOVE_SELF, [=](const QString &filePath){this->ignoreMovedSelf(filePath);});

    QThreadPool::globalInstance()->start(d->runable);

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [=](){
        d->runable->stop();
        if (d->inotifyFD != -1) {
            if ( 0 == close(d->inotifyFD) ) {
                qInfo() << "close inotify fd";
            } else {
                qCritical() << "Failed, close inotify fd error";
            }
        }
        QThreadPool::globalInstance()->waitForDone(3000);
    });
}

Inotify::~Inotify()
{
    qInfo() << "class Inotify destoryed";

    for(auto wd : d->watchers.keys()) {
        inotify_rm_watch(d->inotifyFD, wd);
    }

    if (d->inotifyFD > 0) {
        close(d->inotifyFD);
    }

    if (d) {
        if (d->runable) {
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

void Inotify::addPath(const QString &path)
{
    QtConcurrent::run([=](){

        QWriteLocker lock(&rwLock);

        if (d->inotifyFD == -1)
            return ;

        int watcherID = inotify_add_watch(d->inotifyFD, path.toLatin1(),
                                          IN_MODIFY| IN_OPEN| IN_CLOSE| IN_CREATE|
                                          IN_MOVED_TO| IN_MOVED_FROM| IN_MOVE_SELF| IN_MOVE|
                                          IN_DELETE| IN_DELETE_SELF);

        if (watcherID == -1) {
            qInfo() << "Failed, Create watcher from called inotify_add_watch";
            return ;
        }

        d->watchers[watcherID] = path;

        return ;
    });
}

void Inotify::removePath(const QString &path)
{
    QtConcurrent::run([=](){
        QWriteLocker lock(&rwLock);

        int wd = d->watchers.key(path);
        if (wd < 0)
            return;

        d->watchers.remove(wd);
    });
}

void Inotify::addIgnorePath(const QString &path)
{
    QtConcurrent::run([=](){
        QWriteLocker lock(&rwLock);
        d->ignores.append(path);
    });
}

void Inotify::removeIgnorePath(const QString &path)
{
    QtConcurrent::run([=](){
        QWriteLocker lock(&rwLock);
        d->ignores.removeOne(path);
    });
}

void PollRunable::run()
{
    if (!watcherHash || !ignores)
        return;

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
                QReadLocker rlock(&rwLock);
                for (;;) {
                    ssize_t len = read(inotifyFd, buf, sizeof(buf));
                    if (len == -1 && errno != EAGAIN) {
                        continue;
                    }
                    if (len <= 0)
                        break;
                    for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {

                        event = (const struct inotify_event *) ptr;

                        QString filePath = watcherHash->value(event->wd);
                        if (filePath.isEmpty())
                            continue;

                        if (ignores->contains(filePath)){ // in ignore

                            if (event->mask & IN_OPEN) {
                                auto icb = ignoreCallbacks[typeMapping.key(IN_OPEN)];
                                if (icb) icb(filePath);
                            }
                            if (event->mask & IN_CLOSE) {
                                auto icb = ignoreCallbacks[typeMapping.key(IN_CLOSE)];
                                if (icb) icb(filePath);
                            }
                            if (event->mask & IN_CREATE) {
                                auto icb = ignoreCallbacks[typeMapping.key(IN_CREATE)];
                                if (icb) icb(filePath);
                            }
                            if (event->mask & IN_DELETE) {
                                auto icb = ignoreCallbacks[typeMapping.key(IN_DELETE)];
                                if (icb) icb(filePath);
                            }
                            if (event->mask & IN_MOVE) {
                                auto icb = ignoreCallbacks[typeMapping.key(IN_MOVE)];
                                if (icb) icb(filePath);
                            }
                            if (event->mask & IN_MODIFY) {
                                auto icb = ignoreCallbacks[typeMapping.key(IN_MODIFY)];
                                if (icb) icb(filePath);
                            }
                            if (event->mask & IN_DELETE_SELF) {
                                auto icb  = ignoreCallbacks[typeMapping.key(IN_DELETE_SELF)];
                                if (icb) icb(filePath);
                            }
                            if (event->mask & IN_MOVED_TO) {
                                auto icb = ignoreCallbacks[typeMapping.key(IN_MOVED_TO)];
                                if (icb) icb(filePath);
                            }
                            if (event->mask & IN_MOVED_FROM) {
                                auto icb = ignoreCallbacks[typeMapping.key(IN_MOVED_FROM)];
                                if (icb) icb(filePath);
                            }
                            if (event->mask & IN_MOVE_SELF) {
                                auto icb = ignoreCallbacks[typeMapping.key(IN_MOVE_SELF)];
                                if (icb) icb(filePath);
                            }

                        } else if (watcherHash->values().contains(filePath)) { // in watcher

                            if (event->mask & IN_OPEN) {
                                auto cb = callbacks[typeMapping.key(IN_OPEN)];
                                if (cb) cb(filePath);
                            }
                            if (event->mask & IN_CLOSE) {
                                auto cb = callbacks[typeMapping.key(IN_CLOSE)];
                                if (cb) cb(filePath);
                            }
                            if (event->mask & IN_CREATE) {
                                auto cb = callbacks[typeMapping.key(IN_CREATE)];
                                if (cb) cb(filePath);
                            }
                            if (event->mask & IN_DELETE) {
                                auto cb = callbacks[typeMapping.key(IN_DELETE)];
                                if (cb) cb(filePath);
                            }
                            if (event->mask & IN_MOVE) {
                                auto cb = callbacks[typeMapping.key(IN_MOVE)];
                                if (cb) cb(filePath);
                            }
                            if (event->mask & IN_MODIFY) {
                                auto cb = callbacks[typeMapping.key(IN_MODIFY)];
                                if (cb) cb(filePath);
                            }
                            if (event->mask & IN_DELETE_SELF) {
                                auto cb = callbacks[typeMapping.key(IN_DELETE_SELF)];
                                if (cb) cb(filePath);
                            }
                            if (event->mask & IN_MOVED_TO) {
                                auto cb = callbacks[typeMapping.key(IN_MOVED_TO)];
                                if (cb) cb(filePath);
                            }
                            if (event->mask & IN_MOVED_FROM) {
                                auto cb = callbacks[typeMapping.key(IN_MOVED_FROM)];
                                if (cb) cb(filePath);
                            }
                            if (event->mask & IN_MOVE_SELF) {
                                auto cb = callbacks[typeMapping.key(IN_MOVE_SELF)];
                                if (cb) cb(filePath);
                            }
                        } else {
                            qCritical() << "Inotify file path error" << filePath;
                        }

                        /* Print type of filesystem object. */
                        // if (event->mask & IN_ISDIR)
                        // printf(" [directory]\n");
                        // else
                        // printf(" [file]\n");
                    }
                } // for read event
            }
        }
    }
}
