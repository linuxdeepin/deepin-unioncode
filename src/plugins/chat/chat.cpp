// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chat.h"
#include "option/optionchatgenerator.h"
#include "widgets/chatwidget.h"
#include "chatmanager.h"
#include "eventreceiver.h"

#include "common/common.h"
#include "services/window/windowservice.h"
#include "services/option/optionservice.h"
#include "services/option/optiondatastruct.h"
#include "services/editor/editorservice.h"
#include "copilot.h"

#include "base/abstractwidget.h"
#include "base/abstractaction.h"
#include "base/abstractmenu.h"

#include <QAction>
#include <QIcon>

using namespace dpfservice;

void ChatPlugin::initialize()
{
}

bool ChatPlugin::start()
{
    auto windowService = dpfGetService(dpfservice::WindowService);
    if (windowService) {
        auto Chat = new ChatWidget;
        auto ChatImpl = new AbstractWidget(Chat);
        windowService->addWidgetRightspace(MWNA_CHAT, ChatImpl, "");
    }

    auto optionService = dpfGetService(dpfservice::OptionService);
    if (optionService) {
        optionService->implGenerator<OptionChatGenerator>(option::GROUP_AI, OptionChatGenerator::kitName());
    }

    Copilot::instance();
    ChatManager::instance()->checkCondaInstalled();

    connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, [=] {
        QTimer::singleShot(5000, windowService, [=] {
#ifdef SUPPORTMINIFORGE
            if (!ChatManager::instance()->condaHasInstalled()) {
                QStringList actions { "ai_rag_install", ChatPlugin::tr("Install") };
                windowService->notify(0, "AI", ChatPlugin::tr("Install a Python Conda virtual environment for using the file indexing feature.\
                         Without it, there may be abnormalities in the @codebase and some AI functionalities."),
                                      actions);
            }
#endif
        });
    });

    using namespace std::placeholders;
    auto aiService = dpfGetService(dpfservice::AiService);
    aiService->generateRag = std::bind(&ChatManager::generateRag, ChatManager::instance(), _1);
    aiService->query = std::bind(&ChatManager::query, ChatManager::instance(), _1, _2, _3);
    aiService->chatWithAi = std::bind(&ChatManager::requestAsync, ChatManager::instance(), _1);

    auto editSrv = dpfGetService(EditorService);
    editSrv->registerDiagnosticRepairTool("AI", std::bind(&ChatManager::repairDiagnostic, ChatManager::instance(), _1));

    return true;
}

dpf::Plugin::ShutdownFlag ChatPlugin::stop()
{
    return Sync;
}
