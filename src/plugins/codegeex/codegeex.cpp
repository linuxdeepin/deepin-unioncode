// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeex.h"
#include "askpage/askpage.h"
#include "option/optioncodegeexgenerator.h"
#include "widgets/codegeexwidget.h"
#include "codegeexmanager.h"

#include "common/common.h"
#include "services/window/windowservice.h"
#include "services/option/optionservice.h"
#include "copilot.h"

#include "base/abstractwidget.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractmenu.h"

#include <QAction>
#include <QIcon>

void CodeGeex::initialize()
{
}

bool CodeGeex::start()
{
    auto windowService = dpfGetService(dpfservice::WindowService);
    if (windowService) {
        QString title = "CodeGeex";

        // Add widget to left bar
        if (windowService->addCentralNavigation) {
//            auto askPage = new AskPage();
//            windowService->addWidgetWorkspace(title,  new AbstractWidget(askPage));

            auto codeGeeXWidget = new CodeGeeXWidget();
            windowService->addWidgetWorkspace(title,  new AbstractWidget(codeGeeXWidget));
        }
    }

    auto optionService = dpfGetService(dpfservice::OptionService);
    if (optionService) {
        optionService->implGenerator<OptionCodeGeeXGenerator>(OptionCodeGeeXGenerator::kitName());
    }

    Copilot::instance();

    return true;
}

dpf::Plugin::ShutdownFlag CodeGeex::stop()
{
    return Sync;
}
