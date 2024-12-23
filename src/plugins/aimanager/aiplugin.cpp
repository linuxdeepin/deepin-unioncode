// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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
}

bool AiPlugin::start()
{    
    using namespace dpfservice;
    auto aiService = dpfGetService(AiService);
    auto impl = AiManager::instance();
    using namespace std::placeholders;
    aiService->getAllModel = std::bind(&AiManager::getAllModel, impl);
    aiService->getLLM = std::bind(&AiManager::getLLM, impl, _1);

    auto optionService = dpfGetService(dpfservice::OptionService);
    if (optionService) {
// TODO:uncomment the code when everything is ok 
//        optionService->implGenerator<OptionCustomModelsGenerator>(option::GROUP_AI, OptionCustomModelsGenerator::kitName());
    }

    return true;
}

dpf::Plugin::ShutdownFlag AiPlugin::stop()
{
    return Sync;
}
