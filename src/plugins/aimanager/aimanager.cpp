// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aimanager.h"
#include "services/ai/aiservice.h"
#include "openai/openaicompatiblellm.h"
#include "openai/openaicompletionprovider.h"
#include "services/option/optionmanager.h"
#include "services/editor/editorservice.h"
#include "option/detailwidget.h"
#include "common/util/eventdefinitions.h"
#include "codegeex/codegeexllm.h"
#include "codegeex/codegeexcompletionprovider.h"

#include <QMap>

static const char *kCodeGeeXModelPath = "https://codegeex.cn/prod/code/chatCodeSseV3/chat";

using namespace dpfservice;

class AiManagerPrivate
{
public:
    QList<LLMInfo> models;
    CodeGeeXCompletionProvider *cgcProvider = nullptr;
    OpenAiCompletionProvider *oacProvider = nullptr;
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
    initCompleteProvider();
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
        } else if (info.type == LLMType::ZHIPU_CODEGEEX) {
            auto llm = new CodeGeeXLLM(this);
            llm->setModelName(info.modelName);
            llm->setModelPath(info.modelPath);
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
    auto currentModels = d->models;
    bool changed = false;
    d->models.clear();

    QMap<QString, QVariant> map = OptionManager::getInstance()->getValue(kCATEGORY_CUSTOMMODELS, kCATEGORY_OPTIONKEY).toMap();
    auto LLMs = map.value(kCATEGORY_CUSTOMMODELS);
    if (LLMs.toList().size() != currentModels.size())
        changed = true;

    for (auto llmInfo : LLMs.toList()) {
        LLMInfo info = LLMInfo::fromVariantMap(llmInfo.toMap());
        if (!currentModels.contains(info))
            changed = true;
        appendModel(info);
    }

    for (LLMInfo defaultLLM : getDefaultLLM()) {
        if (!d->models.contains(defaultLLM))
            appendModel(defaultLLM);
    }

    if (changed)
        ai.LLMChanged();

    if (!map.value(kCATEGORY_AUTO_COMPLETE).isValid()) {
        d->cgcProvider->setInlineCompletionEnabled(false);
        d->oacProvider->setInlineCompletionEnabled(false);
        return;
    }

    auto currentCompleteLLMInfo = LLMInfo::fromVariantMap(map.value(kCATEGORY_AUTO_COMPLETE).toMap());
    if (currentCompleteLLMInfo.type == LLMType::OPENAI) {
        d->oacProvider->setInlineCompletionEnabled(true);
        d->oacProvider->setLLM(getLLM(currentCompleteLLMInfo));
        d->cgcProvider->setInlineCompletionEnabled(false); // codegeex completion provider use default url
    } else if (currentCompleteLLMInfo.type == LLMType::ZHIPU_CODEGEEX) {
        d->cgcProvider->setInlineCompletionEnabled(true);
        d->oacProvider->setInlineCompletionEnabled(false);
    }
}

void AiManager::initCompleteProvider()
{
    d->cgcProvider = new CodeGeeXCompletionProvider(this);
    d->oacProvider = new OpenAiCompletionProvider(this);
    d->cgcProvider->setInlineCompletionEnabled(false);
    d->oacProvider->setInlineCompletionEnabled(false);

    connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, this, [=] {
        auto editorService = dpfGetService(EditorService);
        d->cgcProvider->setInlineCompletionEnabled(true);
        editorService->registerInlineCompletionProvider(d->cgcProvider);
        editorService->registerInlineCompletionProvider(d->oacProvider);
    }, Qt::DirectConnection);
}

QList<LLMInfo> AiManager::getDefaultLLM()
{
    LLMInfo liteInfo;
    liteInfo.icon = QIcon::fromTheme("codegeex_model_lite");
    liteInfo.modelName = CodeGeeXChatModelLite;
    liteInfo.modelPath = kCodeGeeXModelPath;
    liteInfo.type = LLMType::ZHIPU_CODEGEEX;

    LLMInfo proInfo;
    proInfo.icon = QIcon::fromTheme("codegeex_model_pro");
    proInfo.modelName = CodeGeeXChatModelPro;
    proInfo.modelPath = kCodeGeeXModelPath;
    proInfo.type = LLMType::ZHIPU_CODEGEEX;

    return { liteInfo, proInfo };
}
