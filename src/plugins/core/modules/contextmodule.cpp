// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contextmodule.h"
#include "find/placeholdermanager.h"
#include "uicontroller/controller.h"

#include "common/widget/outputpane.h"
#include "common/find/outputdocumentfind.h"

#include <QLayout>

void ContextModule::initialize(Controller *_uiController)
{
    AbstractModule::initialize(_uiController);

    auto outputWidget = new AbstractWidget(OutputPane::instance());
    auto docFind = new OutputDocumentFind(OutputPane::instance());
    if (auto holder = PlaceHolderManager::instance()->createPlaceHolder(OutputPane::instance(), docFind))
        OutputPane::instance()->layout()->addWidget(holder);

    uiController->addContextWidget(tr("&Application Output"), outputWidget, true);
}
