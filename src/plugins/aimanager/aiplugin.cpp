// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aiplugin.h"
#include "aimanager.h"
#include "option/optioncustommodelsgenerator.h"
#include "openai/openaicompatiblellm.h"
#include "openai/openaicompatibleconversation.h"
#include "services/ai/aiservice.h"
#include "services/option/optionservice.h"
#include "services/option/optiondatastruct.h"

#include <QAction>
#include <QIcon>

void AiPlugin::initialize()
{
    // load Ai service.
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(dpfservice::AiService::name(), &errStr)) {
        qCritical() << errStr;
    }
}

bool AiPlugin::start()
{    
    using namespace dpfservice;
    auto aiService = dpfGetService(AiService);
    if (!aiService)
        return false;

    auto impl = AiManager::instance();
    using namespace std::placeholders;
    aiService->getAllModel = std::bind(&AiManager::getAllModel, impl);
    aiService->getLLM = std::bind(&AiManager::getLLM, impl, _1);
    aiService->getCodeGeeXLLMLite = []() -> LLMInfo {
        auto defaultLLMs = AiManager::instance()->getDefaultLLM();
        for (auto llm : defaultLLMs) {
            if (llm.modelName == CodeGeeXChatModelLite)
                return llm;
        }
    };
    aiService->getCodeGeeXLLMPro = []() -> LLMInfo {
        auto defaultLLMs = AiManager::instance()->getDefaultLLM();
        for (auto llm : defaultLLMs) {
            if (llm.modelName == CodeGeeXChatModelPro)
                return llm;
        }
    };

    auto optionService = dpfGetService(dpfservice::OptionService);
    if (optionService) {
        optionService->implGenerator<OptionCustomModelsGenerator>(option::GROUP_AI, OptionCustomModelsGenerator::kitName());
    }

    return true;
}

dpf::Plugin::ShutdownFlag AiPlugin::stop()
{
    return Sync;
}
