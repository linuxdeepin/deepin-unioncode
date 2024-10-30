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
#include "copilot.h"

#include "base/abstractwidget.h"
#include "base/abstractaction.h"
#include "base/abstractmenu.h"

#include <QAction>
#include <QIcon>

using namespace dpfservice;

void CodeGeex::initialize()
{
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
        optionService->implGenerator<OptionCodeGeeXGenerator>(QObject::tr("AI"), OptionCodeGeeXGenerator::kitName());
    }

    Copilot::instance();
    CodeGeeXManager::instance()->checkCondaInstalled();

    connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, [=] {
        QTimer::singleShot(5000, windowService, [=] {
            bool ret = CodeGeeXManager::instance()->isLoggedIn();
            if (!ret) {
                QStringList actions { "codegeex_login_default", CodeGeex::tr("Login") };
                windowService->notify(0, "CodeGeex", CodeGeex::tr("Please login to use CodeGeeX."), actions);
            }
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

    return true;
}

dpf::Plugin::ShutdownFlag CodeGeex::stop()
{
    return Sync;
}
