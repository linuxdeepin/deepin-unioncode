/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "binarytools.h"
#include "mainframe/binarytoolsdialog.h"

#include "common/common.h"
#include "base/abstractaction.h"
#include "services/window/windowservice.h"

void BinaryTools::initialize()
{
}

bool BinaryTools::start()
{
    qInfo() << __FUNCTION__;
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    if (windowService) {
        auto action = new QAction(MWMTA_BINARY_TOOLS);
        ActionManager::getInstance()->registerAction(action, "Tools.Binary", action->text(), QKeySequence());
        windowService->addAction(MWM_TOOLS, new AbstractAction(action));

        QObject::connect(action, &QAction::triggered, [=](){
           BinaryToolsDialog dlg;
           dlg.exec();
        });
    }

    return true;
}

dpf::Plugin::ShutdownFlag BinaryTools::stop()
{
    return Sync;
}
