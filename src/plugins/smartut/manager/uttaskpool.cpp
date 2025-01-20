// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uttaskpool.h"
#include "smartutmanager.h"
#include "utils/utils.h"

#include <QEventLoop>
#include <QFile>
#include <QDir>

constexpr int kMaxModelCount { 10 };

using namespace std::placeholders;

UTTaskPool::UTTaskPool(QObject *parent)
    : QObject(parent)
{
}

UTTaskPool::~UTTaskPool()
{
    stop(false);
    for (auto iter = busyModels.cbegin(); iter != busyModels.cend(); ++iter) {
        qDeleteAll(iter.value());
    }
    busyModels.clear();

    for (auto iter = idleModels.cbegin(); iter != idleModels.cend(); ++iter) {
        qDeleteAll(iter.value());
    }
    idleModels.clear();
}

void UTTaskPool::addGenerateTask(const Task &task)
{
    if (!idleModels.contains(task.model))
        createModels(task.model);

    if (idleModels.value(task.model).isEmpty()) {
        taskQueue.enqueue(task);
    } else {
        auto llm = idleModels[task.model].takeFirst();
        busyModels[task.model].append(llm);
        llmItemMap.insert(llm, task.item);
        auto handler = std::bind(&UTTaskPool::handleReceiveResult, this, task.item, _1, _2);
        Q_EMIT started(task.item);

        const QString &tstFW = SmartUTManager::instance()->utSetting()->value(kActiveGroup, kActiveTestFramework).toString();
        if (Utils::isCMakeFile(task.item->itemNode->filePath())) {
            const auto &cmakePrompt = Utils::createCMakePrompt(tstFW);
            llm->request(Utils::createRequestPrompt(task.item->itemNode->asFileNode(), "", cmakePrompt), handler);
        } else {
            const auto &userPrompt = SmartUTManager::instance()->userPrompt(tstFW);
            llm->request(Utils::createRequestPrompt(task.item->itemNode->asFileNode(), "", userPrompt), handler);
        }
    }
}

void UTTaskPool::stop(bool notify)
{
    if (notify) {
        for (const auto &task : taskQueue) {
            Q_EMIT stoped(task.item);
        }
    }
    taskQueue.clear();

    while (!llmItemMap.isEmpty()) {
        if (notify)
            Q_EMIT stoped(llmItemMap.first());
        llmItemMap.firstKey()->cancel();
    }
}

void UTTaskPool::stop(NodeItem *item)
{
    if (item->state == Generating) {
        if (auto llm = llmItemMap.key(item)) {
            llm->cancel();
            Q_EMIT stoped(item);
        }
    } else {
        auto iter = std::find_if(taskQueue.cbegin(), taskQueue.cend(),
                                 [item](const Task &t) {
                                     return item == t.item;
                                 });

        if (iter != taskQueue.cend()) {
            taskQueue.removeOne(*iter);
            Q_EMIT stoped(item);
        }
    }
}

void UTTaskPool::handleReceiveResult(NodeItem *item, const QString &data, AbstractLLM::ResponseState state)
{
    switch (state) {
    case AbstractLLM::Receiving: {
        item->userCache += data;
    } break;
    case AbstractLLM::Success: {
        QFileInfo info(item->itemNode->filePath());
        if (!QFile::exists(info.absolutePath()))
            QDir().mkpath(info.absolutePath());

        QFile file(item->itemNode->filePath());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            const auto &codeList = Utils::queryCodePart(item->userCache, Utils::isCMakeFile(item->itemNode->filePath()) ? "cmake" : "cpp");
            for (const auto &code : codeList) {
                out << code;
            }
            file.close();
        }
    } break;
    default:
        break;
    }

    if (state != AbstractLLM::Receiving) {
        item->userCache.clear();
        if (state == AbstractLLM::Failed)
            item->userCache = data;
        Q_EMIT finished(item, state == AbstractLLM::Success ? Completed : Failed);
    }
}

void UTTaskPool::handleModelStateChanged(AbstractLLM *llm)
{
    if (llm->modelState() == AbstractLLM::Idle) {
        busyModels[llm->modelName()].removeOne(llm);
        idleModels[llm->modelName()].append(llm);
        llmItemMap.remove(llm);
        if (!taskQueue.isEmpty())
            addGenerateTask(taskQueue.dequeue());
    }
}

bool UTTaskPool::createModels(const QString &model)
{
    QList<AbstractLLM *> models;
    for (int i = 0; i < kMaxModelCount; ++i) {
        auto llm = SmartUTManager::instance()->findModel(model);
        if (llm) {
            models << llm;
            connect(llm, &AbstractLLM::modelStateChanged, this, std::bind(&UTTaskPool::handleModelStateChanged, this, llm));
        }
    }

    if (!models.isEmpty())
        idleModels.insert(model, models);
}
