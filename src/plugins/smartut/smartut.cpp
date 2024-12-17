// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "smartut.h"
#include "gui/smartutwidget.h"

#include "base/abstractwidget.h"
#include "services/window/windowservice.h"

DWIDGET_USE_NAMESPACE

void SmartUT::initialize()
{
}

bool SmartUT::start()
{
    auto windowSrv = dpfGetService(dpfservice::WindowService);
    Q_ASSERT(windowSrv);

    auto widget = new SmartUTWidget;
    auto widgetImpl = new AbstractWidget(widget);
    windowSrv->addWidgetRightspace("SmartUT", widgetImpl, "");
    
    settingBtn = new DToolButton(widget);
    settingBtn->setIconSize({16,16});
    settingBtn->setIcon(QIcon::fromTheme("uc_settings"));
    connect(settingBtn, &DToolButton::clicked, widget, &SmartUTWidget::showSettingDialog);
    windowSrv->registerToolBtnToRightspaceWidget(settingBtn, "SmartUT");

    return true;
}

dpf::Plugin::ShutdownFlag SmartUT::stop()
{
    return Sync;
}
