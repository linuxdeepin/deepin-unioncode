// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contextmodule.h"
#include "find/placeholdermanager.h"
#include "uicontroller/controller.h"

#include "common/widget/appoutputpane.h"
#include "common/find/outputdocumentfind.h"

#include <QLayout>

void ContextModule::initialize(Controller *_uiController)
{
    AbstractModule::initialize(_uiController);

    auto outputWidget = new AbstractWidget(AppOutputPane::instance());
    auto docFind = new OutputDocumentFind(AppOutputPane::instance()->defaultPane());
    if (auto holder = PlaceHolderManager::instance()->createPlaceHolder(AppOutputPane::instance()->defaultPane(), docFind))
        AppOutputPane::instance()->defaultPane()->layout()->addWidget(holder);

    connect(AppOutputPane::instance(), &AppOutputPane::paneCreated, this, [=](const QString &id){
        auto pane = AppOutputPane::instance()->getOutputPaneById(id);
        if (pane == AppOutputPane::instance()->defaultPane())
            return;
        auto docFind = new OutputDocumentFind(pane);
        if (auto holder = PlaceHolderManager::instance()->createPlaceHolder(pane, docFind))
            pane->layout()->addWidget(holder);
    });

    uiController->addContextWidget(tr("&Application Output"), outputWidget, true);
}
