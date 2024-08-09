// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "findplugin.h"

#include "services/window/windowservice.h"
#include "services/editor/editorservice.h"
#include "gui/advancedsearchwidget.h"

#include "common/common.h"
#include "base/abstractmenu.h"
#include "base/abstractwidget.h"

#include <DMenu>
#include <QAction>

using namespace dpfservice;

void FindPlugin::initialize()
{
    qInfo() << __FUNCTION__;
}

bool FindPlugin::start()
{
    qInfo() << __FUNCTION__;

    auto &ctx = dpfInstance.serviceContext();
    windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService) {
        qCritical() << "Failed, can't found window service";
        abort();
    }

    registerShortcut();
    registerToSidebar();

    return true;
}

void FindPlugin::switchToSearch()
{
    windowService->raiseMode(CM_EDIT);
    windowService->showWidgetAtPosition(MWNA_ADVANCEDSEARCH, Position::Left, true);
}

void FindPlugin::registerShortcut()
{
    QAction *advancedFindAction = new QAction(this);
    auto advancedFindActionImpl = new AbstractAction(advancedFindAction);
    advancedFindActionImpl->setShortCutInfo("Edit.Advanced.Find",
                                            tr("Advanced Find"), QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key_F));
    connect(advancedFindAction, &QAction::triggered, qApp, [=] {
        auto editSrv = dpfGetService(EditorService);
        const auto &selectedText = editSrv->getSelectedText();
        if (!selectedText.isEmpty())
            advSearchWidget->setSearchText(selectedText);
        windowService->switchWidgetNavigation(MWNA_ADVANCEDSEARCH);
    });
    windowService->addAction("&Edit", advancedFindActionImpl);
}

dpf::Plugin::ShutdownFlag FindPlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}

void FindPlugin::registerToSidebar()
{
    QAction *action = new QAction(MWNA_ADVANCEDSEARCH, this);
    action->setIcon(QIcon::fromTheme("search"));
    auto actionImpl = new AbstractAction(action);
    windowService->addNavigationItem(actionImpl, Priority::highest);

    std::function<AbstractWidget *()> findCreator = []()->AbstractWidget * {
        auto advancedSearchWidget = new AdvancedSearchWidget();
        advancedSearchWidget->initOperator();
        return new AbstractWidget(advancedSearchWidget);
    };

    windowService->registerWidgetCreator(MWNA_ADVANCEDSEARCH, findCreator);
    
    windowService->setDockHeaderName(MWNA_ADVANCEDSEARCH, tr("ADVANCED SEARCH"));
    windowService->bindWidgetToNavigation(MWNA_ADVANCEDSEARCH, actionImpl);

    connect(action, &QAction::triggered, this, &FindPlugin::switchToSearch, Qt::DirectConnection);
}
