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

            windowService->addNavigationItem(new AbstractAction(action));

            auto codeGeex = new CodeGeeXWidget;
            connect(action, &QAction::triggered, this, [=](){
                windowService->raiseMode(CM_EDIT);
                windowService->replaceWidget(MWNA_CODEGEEX, new AbstractWidget(codeGeex), Position::Left);
            }, Qt::DirectConnection);
        }
    }

#if 0 // option not used.
    auto optionService = dpfGetService(dpfservice::OptionService);
    if (optionService) {
        optionService->implGenerator<OptionCodeGeeXGenerator>(OptionCodeGeeXGenerator::kitName());
    }
#endif

    Copilot::instance();

    return true;
}

dpf::Plugin::ShutdownFlag CodeGeex::stop()
{
    return Sync;
}
