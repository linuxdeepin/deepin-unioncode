// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aimanager.h"
#include "services/ai/aiservice.h"
#include "openai/openaicompatiblellm.h"
#include "services/option/optionmanager.h"
#include "option/detailwidget.h"

#include <QMap>

using namespace dpfservice;

class AiManagerPrivate
{
public:
    QList<LLMInfo> models;
};

AiManager *AiManager::instance()
{
    static AiManager ins;
    return &ins;
}

AiManager::AiManager(QObject *parent)
: QObject(parent)
, d(new AiManagerPrivate)
{
    readLLMFromOption();
}

AiManager::~AiManager()
{
    delete d;
}

QList<LLMInfo> AiManager::getAllModel()
{
    return d->models;
}

AbstractLLM *AiManager::getLLM(const LLMInfo &info)
{
    if (d->models.contains(info)) {
        if (info.type == LLMType::OPENAI) {
            auto llm = new OpenAiCompatibleLLM(this);
            llm->setModelName(info.modelName);
            llm->setModelPath(info.modelPath);
            if (!info.apikey.isEmpty())
                llm->setApiKey(info.apikey);
            return llm;
        }
    }

    return nullptr;
}

bool AiManager::checkModelValid(const LLMInfo &info, QString *errStr)
{
    if (info.type == LLMType::OPENAI) {
        OpenAiCompatibleLLM llm;
        llm.setModelName(info.modelName);
        llm.setModelPath(info.modelPath);
        llm.setApiKey(info.apikey);
        bool valid = llm.checkValid(errStr);
        return valid;
    }
}

void AiManager::appendModel(const LLMInfo &info)
{
    if (!d->models.contains(info))
        d->models.append(info);
}

void AiManager::removeModel(const LLMInfo &info)
{
    if (d->models.contains(info))
        d->models.removeOne(info);
}

void AiManager::readLLMFromOption()
{
    d->models.clear();
    QMap<QString, QVariant> map = OptionManager::getInstance()->getValue(kCATEGORY_CUSTOMMODELS, kCATEGORY_OPTIONKEY).toMap();
    auto LLMs = map.value(kCATEGORY_CUSTOMMODELS);
    for (auto llmInfo : LLMs.toList()) {
        appendModel(LLMInfo::fromVariantMap(llmInfo.toMap()));
    }
}
