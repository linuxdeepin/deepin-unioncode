// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeex.h"
#include "option/optioncodegeexgenerator.h"
#include "widgets/codegeexwidget.h"
#include "codegeexmanager.h"

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
        if (windowService->addNavigationItem) {
            QAction *action = new QAction(MWNA_CODEGEEX, this);
            action->setIcon(QIcon::fromTheme("codegeex-navigation"));
            auto actionImpl = new AbstractAction(action);
            windowService->addNavigationItem(actionImpl, Priority::medium);

            auto codeGeex = new CodeGeeXWidget;
            auto codeGeexImpl = new AbstractWidget(codeGeex);
            windowService->registerWidget(MWNA_CODEGEEX, codeGeexImpl);
            windowService->setDockHeaderName(MWNA_CODEGEEX, "codeGeex");
            windowService->bindWidgetToNavigation(MWNA_CODEGEEX, actionImpl);

            connect(action, &QAction::triggered, this, [=]() {
                windowService->raiseMode(CM_EDIT);
                windowService->showWidgetAtPosition(MWNA_CODEGEEX, Position::Left, true);
            },
                    Qt::DirectConnection);
        }
    }

    auto optionService = dpfGetService(dpfservice::OptionService);
    if (optionService) {
        optionService->implGenerator<OptionCodeGeeXGenerator>(QObject::tr("AI"), OptionCodeGeeXGenerator::kitName());
    }

    Copilot::instance();

    connect(&dpf::Listener::instance(), &dpf::Listener::pluginsStarted, [=] {
        QTimer::singleShot(5000, windowService, [=] {
            bool ret = CodeGeeXManager::instance()->isLoggedIn();
            if (ret)
                return;

            QStringList actions { "codegeex_login_default", CodeGeex::tr("Login") };
            windowService->notify(0, "CodeGeex", CodeGeex::tr("Please login to use CodeGeeX."), actions);
        });
    });

    return true;
}

dpf::Plugin::ShutdownFlag CodeGeex::stop()
{
    return Sync;
}
