// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTTASKPOOL_H
#define UTTASKPOOL_H

#include "common/itemnode.h"

#include "base/ai/abstractllm.h"

#include <QQueue>

class UTTaskPool : public QObject
{
    Q_OBJECT
public:
    struct Task
    {
        QString model;
        NodeItem *item;

        bool operator==(const Task &task) const
        {
            return model == task.model && item == task.item;
        }
    };

    explicit UTTaskPool(QObject *parent = nullptr);
    ~UTTaskPool();

    void addGenerateTask(const Task &task);
    void stop(bool notify = true);
    void stop(NodeItem *item);

public Q_SLOTS:
    void generateUTFile(const NodeItem *item);
    void handleReceiveResult(NodeItem *item, const QString &data, AbstractLLM::ResponseState state);
    void handleModelStateChanged(AbstractLLM *llm);

Q_SIGNALS:
    void started(NodeItem *item);
    void finished(NodeItem *item, ItemState state);
    void stoped(NodeItem *item);

private:
    void createModels(const QString &model);

    QQueue<Task> taskQueue;
    QMap<QString, QList<AbstractLLM *>> idleModels;
    QMap<QString, QList<AbstractLLM *>> busyModels;
    QMap<AbstractLLM *, NodeItem *> llmItemMap;
};

#endif   // UTTASKPOOL_H
