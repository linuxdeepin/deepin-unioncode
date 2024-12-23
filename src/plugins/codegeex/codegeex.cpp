// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeex.h"
#include "option/optioncodegeexgenerator.h"
#include "widgets/codegeexwidget.h"
#include "codegeexmanager.h"
#include "eventreceiver.h"

#include "common/common.h"
#include "services/window/windowservice.h"
#include "services/option/optionservice.h"
#include "services/option/optiondatastruct.h"
#include "services/ai/aiservice.h"
#include "copilot.h"

#include "base/abstractwidget.h"
#include "base/abstractaction.h"
#include "base/abstractmenu.h"

#include <QAction>
#include <QIcon>

using namespace dpfservice;

void CodeGeex::initialize()
{
    // load Ai service.
    QString errStr;
    auto &ctx = dpfInstance.serviceContext();
    if (!ctx.load(AiService::name(), &errStr)) {
        qCritical() << errStr;
    }
}

bool CodeGeex::start()
{
    auto windowService = dpfGetService(dpfservice::WindowService);
    if (windowService) {
        auto codeGeex = new CodeGeeXWidget;
        auto codeGeexImpl = new AbstractWidget(codeGeex);
        windowService->addWidgetRightspace(MWNA_CODEGEEX, codeGeexImpl, "");
    }

    auto optionService = dpfGetService(dpfservice::OptionService);
    if (optionService) {
        optionService->implGenerator<OptionCodeGeeXGenerator>(option::GROUP_AI, OptionCodeGeeXGenerator::kitName());
    }

    Copilot::instance();
    CodeGeeXManager::instance()->checkCondaInstalled();

    connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, [=] {
        QTimer::singleShot(5000, windowService, [=] {
#ifdef SUPPORTMINIFORGE
            if (!CodeGeeXManager::instance()->condaHasInstalled()) {
                QStringList actions { "ai_rag_install", CodeGeex::tr("Install") };
                windowService->notify(0, "AI", CodeGeex::tr("Install a Python Conda virtual environment for using the file indexing feature.\
                         Without it, there may be abnormalities in the @codebase and some AI functionalities."),
                                      actions);
            }
#endif
        });
    });

    using namespace std::placeholders;
    auto aiService = dpfGetService(dpfservice::AiService);
    aiService->generateRag = std::bind(&CodeGeeXManager::generateRag, CodeGeeXManager::instance(), _1);
    aiService->query = std::bind(&CodeGeeXManager::query, CodeGeeXManager::instance(), _1, _2, _3);
    aiService->chatWithAi = std::bind(&CodeGeeXManager::requestAsync, CodeGeeXManager::instance(), _1);

    return true;
}

dpf::Plugin::ShutdownFlag CodeGeex::stop()
{
    return Sync;
}
