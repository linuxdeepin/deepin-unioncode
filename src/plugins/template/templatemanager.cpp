// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "templatemanager.h"
#include "wizard/maindialog.h"

#include "services/window/windowservice.h"
#include "base/abstractaction.h"
#include "common/actionmanager/actionmanager.h"

using namespace dpfservice;

class TemplateManagerPrivate
{
    friend class TemplateManager;

    QSharedPointer<QAction> newAction;
};

TemplateManager *TemplateManager::instance()
{
    static TemplateManager ins;
    return &ins;
}

TemplateManager::TemplateManager(QObject *parent)
    : QObject(parent)
    , d(new TemplateManagerPrivate())
{

}

TemplateManager::~TemplateManager()
{
    if (d) {
        delete d;
    }
}

void TemplateManager::initialize()
{
    addMenu();
}

void TemplateManager::addMenu()
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    auto actionInit = [&](QAction *action, QString actionID, QKeySequence key, QString iconFileName){
        ActionManager::getInstance()->registerAction(action, actionID, action->text(), key, iconFileName);
        AbstractAction *actionImpl = new AbstractAction(action);
        windowService->addAction(dpfservice::MWMFA_NEW_FILE_OR_PROJECT, actionImpl);
    };

    d->newAction.reset(new QAction(MWMFA_NEW_FILE_OR_PROJECT));
    actionInit(d->newAction.get(),
               "File.New.FileOrProject",
               QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_O), QString());
    QObject::connect(d->newAction.get(), &QAction::triggered,
                     this, &TemplateManager::newWizard, Qt::DirectConnection);
}

void TemplateManager::newWizard()
{
    MainDialog *mainDlg = new MainDialog();
    mainDlg->exec();
}
