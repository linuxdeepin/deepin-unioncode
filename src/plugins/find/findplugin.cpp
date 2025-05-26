// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "findplugin.h"

#include "services/window/windowservice.h"
#include "services/editor/editorservice.h"
#include "gui/advancedsearchwidget.h"

#include "common/common.h"
#include "common/actionmanager/actioncontainer.h"
#include "base/abstractmenu.h"
#include "base/abstractwidget.h"

#include <DMenu>
#include <QAction>

constexpr char M_FIND[] = "Find.FindMenu";

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
    auto mFind = ActionManager::instance()->actionContainer(M_FIND);
    mFind->insertGroup("Find.FindMenu.Actions", "Find.FindMenu.Advanced");

    QAction *advancedFindAction = new QAction(tr("Advanced Find"), mFind);
    auto cmd = ActionManager::instance()->registerAction(advancedFindAction, "Find.AdvancedFind");
    cmd->setDefaultKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F);
    mFind->addAction(cmd, "Find.FindMenu.Advanced");
    connect(advancedFindAction, &QAction::triggered, qApp, [=] {
        windowService->switchWidgetNavigation(MWNA_ADVANCEDSEARCH);
        if (!advSearchWidget)
            return;

        auto editSrv = dpfGetService(EditorService);
        const auto &selectedText = editSrv->getSelectedText();
        if (!selectedText.isEmpty())
            advSearchWidget->setSearchText(selectedText);
    });
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

    std::function<AbstractWidget *()> findCreator = [this]() -> AbstractWidget * {
        advSearchWidget = new AdvancedSearchWidget();
        advSearchWidget->initOperator();
        return new AbstractWidget(advSearchWidget);
    };

    windowService->registerWidgetCreator(MWNA_ADVANCEDSEARCH, findCreator);
    windowService->setDockHeaderName(MWNA_ADVANCEDSEARCH, tr("ADVANCED SEARCH"));
    windowService->bindWidgetToNavigation(MWNA_ADVANCEDSEARCH, actionImpl);

    connect(action, &QAction::triggered, this, &FindPlugin::switchToSearch, Qt::DirectConnection);
}
