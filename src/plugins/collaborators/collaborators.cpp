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
            QAction *actionSvn = new QAction(MWNA_SVN, this);
            actionSvn->setIcon(QIcon::fromTheme("svn-navigation"));

            windowService->addNavigationItem(new AbstractAction(actionGit), 10);
            windowService->addNavigationItem(new AbstractAction(actionSvn), 10);

            AbstractWidget *gitMainWidgetImpl = new AbstractWidget(CVSkeeper::instance()->gitMainWidget());
            AbstractWidget *svnMainWidgetImpl = new AbstractWidget(CVSkeeper::instance()->svnMainWidget());

            connect(actionGit, &QAction::triggered, this, [=](){
                windowService->replaceWidget(MWNA_GIT,
                                         gitMainWidgetImpl,
                                         Position::FullWindow);
                windowService->hideStatusBar();
            }, Qt::DirectConnection);
            connect(actionSvn, &QAction::triggered, this, [=](){
                windowService->replaceWidget(MWNA_SVN,
                                         svnMainWidgetImpl,
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
