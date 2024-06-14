// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collaborators.h"
#include "services/window/windowservice.h"
#include "mainframe/cvskeeper.h"
#include "base/abstractaction.h"
#include "base/abstractwidget.h"

#include <QAction>

using namespace dpfservice;
void Collaborators::initialize()
{

}

bool Collaborators::start()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        if (windowService->addNavigationItem) {
            QAction *actionGit = new QAction(MWNA_GIT, this);
            actionGit->setIcon(QIcon::fromTheme("git-navigation"));
            
#ifdef ENABLE_SVN // TODO(Any): svn should not contained in this plugin.
            QAction *actionSvn = new QAction(MWNA_SVN, this);
            actionSvn->setIcon(QIcon::fromTheme("svn-navigation"));
            windowService->addNavigationItem(new AbstractAction(actionSvn), Priority::medium);
            AbstractWidget *svnMainWidgetImpl = new AbstractWidget(CVSkeeper::instance()->svnMainWidget());
            windowService->registerWidget(MWNA_SVN, svnMainWidgetImpl);
            connect(actionSvn, &QAction::triggered, this, [=](){
                windowService->replaceWidget(MWNA_SVN,
                                         Position::FullWindow);
                windowService->hideStatusBar();
            }, Qt::DirectConnection);
#endif

            windowService->addNavigationItem(new AbstractAction(actionGit), Priority::medium);
            
            AbstractWidget *gitMainWidgetImpl = new AbstractWidget(CVSkeeper::instance()->gitMainWidget());
           

            windowService->registerWidget(MWNA_GIT, gitMainWidgetImpl);
            

            connect(actionGit, &QAction::triggered, this, [=](){
                windowService->replaceWidget(MWNA_GIT,
                                         Position::FullWindow);
                windowService->hideStatusBar();
            }, Qt::DirectConnection);
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag Collaborators::stop()
{
    return Sync;
}
