// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    if (ins == nullptr) {
        ::ins = new Inotify;
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, [=](){
            delete ::ins;
        });
    }
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
                     Qt::QueuedConnection);
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
    if (qApp->thread() != QThread::currentThread())
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
