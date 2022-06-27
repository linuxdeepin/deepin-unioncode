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
#include "private/inotify_p.h"
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

namespace  {
static Inotify *ins {nullptr};
}

Inotify::Inotify(QObject *parent)
    : QObject(parent)
    , d(new InotifyPrivate(this))
{

}

Inotify::~Inotify()
{
    if (d) {
        delete d;
    }
}

Inotify *Inotify::globalInstance()
{
    if (ins == nullptr)
        ::ins = new Inotify;
    if (qApp->thread() != QThread::currentThread())
        ins->moveToThread(qApp->thread());
    return ins;
}

void Inotify::addPath(const QString &path)
{
    d->addPath(path);
}

void Inotify::removePath(const QString &path)
{
    d->removePath(path);
}

void Inotify::addIgnorePath(const QString &path)
{
    d->ignoreList.append(path);
}

void Inotify::removeIgnorePath(const QString &path)
{
    d->ignoreList.removeOne(path);
}

InotifyPrivate::InotifyPrivate(Inotify *q)
    : q (q)
    , ignoreList()
    , hook(InotifyImpl::createInotify())
{
    QObject::connect(hook, &InotifyHook::inotifyEvent,
                     this, &InotifyPrivate::doInotifyEvent,
                     Qt::ConnectionType::QueuedConnection);
}

InotifyPrivate::~InotifyPrivate()
{
    if (hook)
        InotifyImpl::removeInotify(hook);
}

void InotifyPrivate::addPath(const QString &path)
{
    hook->addPath(path);
}

void InotifyPrivate::removePath(const QString &path)
{
    hook->removePath(path);
}

void InotifyPrivate::doInotifyEvent(InotifyHook::Type type, const QString &path)
{
    qInfo() << qApp->thread() << QThread::currentThread();
    if (ignoreList.contains(path)) {
        if (type == InotifyHook::Type::MODIFY)
            emit q->ignoreModified(path);
        else if (type == InotifyHook::Type::CLOSE)
            emit q->ignoreClosed(path);
        else if (type == InotifyHook::Type::OPEN)
            emit q->ignoreOpened(path);
        else if (type == InotifyHook::Type::MOVE)
            emit q->ignoreMovedSub(path);
        else if (type == InotifyHook::Type::DELETE)
            emit q->ignoreDeletedSub(path);
        else if (type == InotifyHook::Type::CREATE)
            emit q->ignoreCreatedSub(path);
        else if (type == InotifyHook::Type::DELETE_SELF)
            emit q->ignoreDeletedSelf(path);
        else if (type == InotifyHook::Type::MOVE_SELF)
            emit q->ignoreMovedSelf(path);
        return;
    }

    if (type == InotifyHook::Type::MODIFY)
        emit q->modified(path);
    else if (type == InotifyHook::Type::CLOSE)
        emit q->closed(path);
    else if (type == InotifyHook::Type::OPEN)
        emit q->opened(path);
    else if (type == InotifyHook::Type::MOVE)
        emit q->movedSub(path);
    else if (type == InotifyHook::Type::DELETE)
        emit q->deletedSub(path);
    else if (type == InotifyHook::Type::CREATE)
        emit q->createdSub(path);
    else if (type == InotifyHook::Type::DELETE_SELF)
        emit q->deletedSelf(path);
    else if (type == InotifyHook::Type::MOVE_SELF)
        emit q->movedSelf(path);
}
