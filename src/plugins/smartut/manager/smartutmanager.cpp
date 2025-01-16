// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smartutmanager.h"
#include "uttaskpool.h"
#include "utils/utils.h"

#include "common/util/custompaths.h"
#include "services/ai/aiservice.h"
#include "services/project/projectservice.h"

#include <QDir>

using namespace dpfservice;

class SmartUTManagerPrivate
{
public:
    void init();
    QString configPath() const;

    QString chunkPrompt(const QStringList &fileList, const QString &workspace);

public:
    QString errorMsg;
    Settings utSetting;

    AiService *aiSrv { nullptr };
    ProjectService *prjSrv { nullptr };
    UTTaskPool taskPool;
};

void SmartUTManagerPrivate::init()
{
    aiSrv = dpfGetService(AiService);
    prjSrv = dpfGetService(ProjectService);
    Q_ASSERT(aiSrv && prjSrv);

    utSetting.load(":/configure/smartut.json", configPath());
}

QString SmartUTManagerPrivate::configPath() const
{
    return CustomPaths::user(CustomPaths::Flags::Configures)
            + "/SmartUT/smartut.json";
}

QString SmartUTManagerPrivate::chunkPrompt(const QStringList &fileList, const QString &workspace)
{
    const auto &result = aiSrv->query(workspace, "code for unit test", 20);
    return Utils::createChunkPrompt(result);
}

SmartUTManager::SmartUTManager(QObject *parent)
    : QObject(parent),
      d(new SmartUTManagerPrivate)
{
    d->init();
    connect(&d->taskPool, &UTTaskPool::finished, this,
            [this](NodeItem *item, ItemState state) {
                item->state = state;
                Q_EMIT itemStateChanged(item);
            });
    connect(&d->taskPool, &UTTaskPool::started, this,
            [this](NodeItem *item) {
                item->state = Generating;
                Q_EMIT itemStateChanged(item);
            });
    connect(&d->taskPool, &UTTaskPool::stoped, this,
            [this](NodeItem *item) {
                item->state = None;
                Q_EMIT itemStateChanged(item);
            });
}

SmartUTManager::~SmartUTManager()
{
    delete d;
}

SmartUTManager *SmartUTManager::instance()
{
    static SmartUTManager ins;
    return &ins;
}

Settings *SmartUTManager::utSetting()
{
    return &d->utSetting;
}

QStringList SmartUTManager::modelList() const
{
    const auto &models = d->aiSrv->getAllModel();
    QStringList names;
    std::transform(models.cbegin(), models.cend(), std::back_inserter(names),
                   [](const LLMInfo &info) {
                       return info.modelName;
                   });
    return names;
}

QList<ProjectInfo> SmartUTManager::projectList() const
{
    return d->prjSrv->getAllProjectInfo();
}

AbstractLLM *SmartUTManager::findModel(const QString &model)
{
    const auto &models = d->aiSrv->getAllModel();
    auto iter = std::find_if(models.cbegin(), models.cend(),
                             [model](const LLMInfo &info) {
                                 return info.modelName == model;
                             });
    if (iter == models.cend()) {
        d->errorMsg = tr("A model named \"%1\" was not found").arg(model);
        return nullptr;
    }

    auto llm = d->aiSrv->getLLM(*iter);
    //    if (!llm->checkValid(&d->errorMsg)) {
    //        delete llm;
    //        return nullptr;
    //    }

    return llm;
}

QString SmartUTManager::userPrompt(const QString &framework) const
{
    QString prompt = QString("单元测试框架为：%1\n").arg(framework);
    const auto &prompts = d->utSetting.value(kGeneralGroup, kPrompts).toMap();
    const auto &title = d->utSetting.value(kActiveGroup, kActivePrompt).toString();
    const auto &tempFile = d->utSetting.value(kActiveGroup, kActiveTemplate).toString();

    prompt += prompts.value(title, "").toString();
    if (!tempFile.isEmpty() && QFile::exists(tempFile)) {
        QFile file(tempFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return prompt;

        const auto &content = file.readAll();
        if (content.isEmpty())
            return prompt;

        prompt += "\n参考下面提供的内容生成单元测试：\n```cpp\n";
        prompt += content;
        prompt += "\n```";
    }

    return prompt;
}

void SmartUTManager::generateUTFiles(const QString &model, NodeItem *item)
{
    auto checkValid = [](NodeItem *item) {
        return !item->hasChildren()
                && item->itemNode->isFileNodeType()
                && item->state != Ignored
                && item->state != Waiting
                && item->state != Generating;
    };

    if (checkValid(item)) {
        item->state = Waiting;
        item->userCache.clear();
        Q_EMIT itemStateChanged(item);
        d->taskPool.addGenerateTask({ model, item });
    } else if (item->hasChildren()) {
        for (int i = 0; i < item->rowCount(); ++i) {
            generateUTFiles(model, static_cast<NodeItem *>(item->child(i)));
        }
    }
}

void SmartUTManager::runTest(const dpfservice::ProjectInfo &prjInfo)
{
    //TODO:
}

void SmartUTManager::generateCoverageReport(const dpfservice::ProjectInfo &prjInfo)
{
    //TODO:
}

void SmartUTManager::stop()
{
    d->taskPool.stop();
}

void SmartUTManager::stop(NodeItem *item)
{
    auto checkGenerating = [](NodeItem *item) {
        return !item->hasChildren()
                && item->itemNode->isFileNodeType()
                && (item->state == Waiting || item->state == Generating);
    };

    if (checkGenerating(item)) {
        d->taskPool.stop(item);
    } else if (item->hasChildren()) {
        for (int i = 0; i < item->rowCount(); ++i) {
            stop(static_cast<NodeItem *>(item->child(i)));
        }
    }
}

QString SmartUTManager::lastError() const
{
    return d->errorMsg;
}
