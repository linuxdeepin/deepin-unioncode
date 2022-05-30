#include "findplugin.h"

#include "services/window/windowservice.h"
#include "findtoolbar.h"
#include "currentdocumentfind.h"
#include "findtoolwindow.h"
#include "searchresultwindow.h"
#include "common/common.h"
#include "base/abstractmenu.h"
#include "base/abstractwidget.h"

#include <QMenu>
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

    QMenu* editMenu = new QMenu("&Edit");
    QAction* findAction = new QAction();
    QAction* advancedFindAction = new QAction();

    ActionManager::getInstance()->registerAction(findAction, "Edit.Find",
                                                 "Find/Replace", QKeySequence(Qt::Modifier::CTRL | Qt::Key_F));
    ActionManager::getInstance()->registerAction(advancedFindAction, "Edit.Advanced.Find",
                                                 "Advanced Find", QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT |Qt::Key_F));

    editMenu->addAction(findAction);
    editMenu->addAction(advancedFindAction);

    connect(findAction, &QAction::triggered, [=] {
        emit windowService->showFindToolBar();
    });

    connect(advancedFindAction, &QAction::triggered, [=] {
        emit windowService->switchWidgetContext ("&Search Results");
    });

    AbstractMenu * menuImpl = new AbstractMenu(editMenu);
    windowService->addMenu(menuImpl);

    AbstractWidget *widgetImpl = new AbstractWidget(new FindToolWindow());
    emit windowService->addContextWidget("&Search Results", widgetImpl);

    FindToolBar * findToolBar = new FindToolBar(new CurrentDocumentFind());
    AbstractWidget *abstractFindToolBar = new AbstractWidget(findToolBar);
    emit windowService->addFindToolBar(abstractFindToolBar);
    connect(findToolBar, &FindToolBar::advanced, this, &FindPlugin::sendSwitchSearchResult);

    return true;
}

void FindPlugin::sendSwitchSearchResult()
{
    emit windowService->switchWidgetContext ("&Search Results");
}

dpf::Plugin::ShutdownFlag FindPlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}


