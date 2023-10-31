// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "codegeex.h"
#include "askpage/askpage.h"

#include "common/common.h"
#include "services/window/windowservice.h"

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
            auto askPage = new AskPage();
            windowService->addActionNavigation(title, new AbstractAction(new QAction(QIcon(":/CodeGeex/images/navigation.png"), QAction::tr("CodeGeex"))));
            windowService->addCentralNavigation(title, new AbstractCentral(askPage));
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag CodeGeex::stop()
{
    return Sync;
}
