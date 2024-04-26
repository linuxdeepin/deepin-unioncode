// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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
    if (auto holder = PlaceHolderManager::instance()->createPlaceHolder(AppOutputPane::instance(), docFind))
        AppOutputPane::instance()->layout()->addWidget(holder);

    uiController->addContextWidget(tr("&Application Output"), outputWidget, true);
}
