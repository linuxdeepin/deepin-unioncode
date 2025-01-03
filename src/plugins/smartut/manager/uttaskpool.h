// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTTASKPOOL_H
#define UTTASKPOOL_H

#include "common/itemnode.h"

#include "base/ai/abstractllm.h"

#include <QMutex>
#include <QThreadPool>

class Task : public QObject, public QRunnable
{
    Q_OBJECT
public:
    Task() = default;
    explicit Task(const QString &model, NodeItem *item);

    void run() override;

    void handleReceiveResult(NodeItem *item, const QString &data, AbstractLLM::ResponseState state);

Q_SIGNALS:
    void started(NodeItem *item);
    void finished(NodeItem *item, ItemState state);

private:
    QString model;
    NodeItem *item { nullptr };
    QString result;
};

class UTTaskPool : public QObject
{
    Q_OBJECT
public:
    explicit UTTaskPool(QObject *parent = nullptr);
    ~UTTaskPool();

    void addGenerateTask(const QString &model, NodeItem *item);

    void waitForDone();
    void stop();

public Q_SLOTS:
    void generateUTFile(const NodeItem *item);

Q_SIGNALS:
    void started(NodeItem *item);
    void finished(NodeItem *item, ItemState state);

private:
    QThreadPool *threadPool { nullptr };
    QMutex mutex;
    bool isRunning { true };
};

#endif   // UTTASKPOOL_H
