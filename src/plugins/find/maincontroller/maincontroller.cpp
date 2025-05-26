// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maincontroller.h"
#include "task/taskcommander.h"

#include <QHash>
#include <QUuid>

class MainControllerPrivate
{
public:
    ~MainControllerPrivate();

public:
    TaskCommander *currentTask { nullptr };
};

MainControllerPrivate::~MainControllerPrivate()
{
    if (currentTask) {
        currentTask->stop();
        currentTask->deleteSelf();
        currentTask = nullptr;
    }
}

MainController::MainController(QObject *parent)
    : QObject(parent),
      d(new MainControllerPrivate)
{
}

MainController::~MainController()
{
    delete d;
}

bool MainController::search(const SearchParams &params)
{
    stop();
    if (params.keyword.isEmpty())
        return false;

    auto task = new TaskCommander();
    connect(task, &TaskCommander::matched, this, &MainController::matched, Qt::DirectConnection);
    connect(task, &TaskCommander::finished, this, &MainController::searchFinished, Qt::DirectConnection);

    if (task->search(params)) {
        d->currentTask = task;
        return true;
    }

    task->deleteSelf();
    return false;
}

bool MainController::replace(const ReplaceParams &params)
{
    stop();
    auto task = new TaskCommander();
    connect(task, &TaskCommander::finished, this, &MainController::replaceFinished, Qt::DirectConnection);

    if (task->replace(params)) {
        d->currentTask = task;
        return true;
    }

    task->deleteSelf();
    return false;
}

void MainController::stop()
{
    if (!d->currentTask)
        return;

    disconnect(d->currentTask, nullptr, this, nullptr);
    d->currentTask->stop();
    d->currentTask->deleteSelf();
    d->currentTask = nullptr;
}

FindItemList MainController::takeAll()
{
    if (d->currentTask)
        return d->currentTask->takeAll();

    return {};
}
