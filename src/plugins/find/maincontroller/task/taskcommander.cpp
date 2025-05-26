// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskcommander.h"
#include "maincontroller/worker/searchreplaceworker.h"

#include <QThread>
#include <QReadWriteLock>
#include <QSharedPointer>

class TaskCommanderPrivate : public QObject
{
public:
    explicit TaskCommanderPrivate(TaskCommander *qq);
    ~TaskCommanderPrivate();

    void onFinished();
    void onMatched();

public:
    TaskCommander *q;

    QThread workThread;
    QSharedPointer<SearchReplaceWorker> srWorker { nullptr };

    QReadWriteLock rwLock;
    FindItemList resultList;

    volatile bool isWorking = false;
    bool deleted = false;
    bool finished = false;
};

TaskCommanderPrivate::TaskCommanderPrivate(TaskCommander *qq)
    : q(qq),
      srWorker(new SearchReplaceWorker)
{

    connect(srWorker.data(), &SearchReplaceWorker::matched, this, &TaskCommanderPrivate::onMatched, Qt::DirectConnection);
    connect(srWorker.data(), &SearchReplaceWorker::finished, this, &TaskCommanderPrivate::onFinished, Qt::QueuedConnection);

    srWorker->moveToThread(&workThread);
    workThread.start();
}

TaskCommanderPrivate::~TaskCommanderPrivate()
{
    metaObject()->invokeMethod(srWorker.data(), &SearchReplaceWorker::stop);
    workThread.quit();
    workThread.wait();
}

void TaskCommanderPrivate::onFinished()
{
    if (deleted) {
        q->deleteLater();
        disconnect(q, nullptr, nullptr, nullptr);
    } else if (!finished) {
        finished = true;
        emit q->finished();
    }
}

void TaskCommanderPrivate::onMatched()
{
    if (!srWorker->hasItem())
        return;

    auto results = srWorker->takeAll();
    QWriteLocker lk(&rwLock);
    bool isEmpty = resultList.isEmpty();

    resultList += results;
    if (isEmpty)
        QMetaObject::invokeMethod(q, "matched", Qt::QueuedConnection);
}

TaskCommander::TaskCommander(QObject *parent)
    : QObject(parent),
      d(new TaskCommanderPrivate(this))
{
}

bool TaskCommander::search(const SearchParams &params)
{
    if (d->isWorking)
        return false;

    d->isWorking = true;
    QMetaObject::invokeMethod(d->srWorker.data(),
                              "search",
                              Qt::QueuedConnection,
                              Q_ARG(SearchParams, params));

    return true;
}

bool TaskCommander::replace(const ReplaceParams &params)
{
    if (d->isWorking)
        return false;

    QMetaObject::invokeMethod(d->srWorker.data(),
                              "replace",
                              Qt::QueuedConnection,
                              Q_ARG(ReplaceParams, params));

    d->isWorking = true;
    return true;
}

void TaskCommander::stop()
{
    metaObject()->invokeMethod(d->srWorker.data(), &SearchReplaceWorker::stop);
    d->isWorking = false;
}

void TaskCommander::deleteSelf()
{
    if (d->finished)
        delete this;
    else
        d->deleted = true;
}

FindItemList TaskCommander::takeAll()
{
    QReadLocker lk(&d->rwLock);
    return std::move(d->resultList);
}
