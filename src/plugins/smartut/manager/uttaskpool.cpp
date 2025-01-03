// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uttaskpool.h"
#include "smartutmanager.h"
#include "utils/utils.h"

#include <QEventLoop>
#include <QFile>
#include <QDir>

constexpr int kMaxThreadCount { 10 };

Task::Task(const QString &model, NodeItem *item)
    : model(model),
      item(item)
{
}

void Task::run()
{
    Q_EMIT started(item);
    QEventLoop loop;
    auto llm = SmartUTManager::instance()->findModel(model);
    if (!llm) {
        Q_EMIT finished(item, Failed);
        return;
    }

    llm->setStream(true);
    connect(llm, &AbstractLLM::dataReceived, this,
            [&loop, this](const QString &data, AbstractLLM::ResponseState state) {
                handleReceiveResult(item, data, state);
                if (state != AbstractLLM::Receiving) {
                    Q_EMIT finished(item, state == AbstractLLM::Success ? Completed : Failed);
                    loop.quit();
                }
            });
    const QString &tstFW = SmartUTManager::instance()->utSetting()->value(kActiveGroup, kActiveTestFramework).toString();
    if (Utils::isCMakeFile(item->itemNode->filePath())) {
        const auto &cmakePrompt = Utils::createCMakePrompt(tstFW);
        llm->request(Utils::createRequestPrompt(item->itemNode->asFileNode(), "", cmakePrompt));
    } else {
        const auto &userPrompt = SmartUTManager::instance()->userPrompt(tstFW);
        llm->request(Utils::createRequestPrompt(item->itemNode->asFileNode(), "", userPrompt));
    }

    loop.exec();
    llm->deleteLater();
}

void Task::handleReceiveResult(NodeItem *item, const QString &data, AbstractLLM::ResponseState state)
{
    switch (state) {
    case AbstractLLM::Receiving:
        result += data;
        break;
    case AbstractLLM::Success: {
        QFileInfo info(item->itemNode->filePath());
        if (!QFile::exists(info.absolutePath()))
            QDir().mkpath(info.absolutePath());

        QFile file(item->itemNode->filePath());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            const auto &codeList = Utils::queryCodePart(result, Utils::isCMakeFile(item->itemNode->filePath()) ? "cmake" : "cpp");
            for (const auto &code : codeList) {
                out << code;
            }
            file.close();
        }
    } break;
    default:
        break;
    }
}

UTTaskPool::UTTaskPool(QObject *parent)
    : QObject(parent),
      threadPool(new QThreadPool(this))
{
    threadPool->setMaxThreadCount(kMaxThreadCount);
}

UTTaskPool::~UTTaskPool()
{
    stop();
    threadPool->waitForDone();
}

void UTTaskPool::addGenerateTask(const QString &model, NodeItem *item)
{
    QMutexLocker locker(&mutex);
    if (!isRunning)
        return;

    Task *task = new Task(model, item);
    connect(task, &Task::finished, this, &UTTaskPool::finished);
    connect(task, &Task::started, this, &UTTaskPool::started);
    task->setAutoDelete(true);
    threadPool->start(task);
}

void UTTaskPool::waitForDone()
{
    threadPool->waitForDone();
}

void UTTaskPool::stop()
{
    QMutexLocker locker(&mutex);
    isRunning = false;
    threadPool->clear();
}
