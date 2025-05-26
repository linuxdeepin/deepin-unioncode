// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "templatemanager.h"
#include "wizard/maindialog.h"

#include "services/window/windowservice.h"
#include "base/abstractaction.h"
#include "common/actionmanager/actionmanager.h"
#include "common/util/utils.h"

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

    auto mFile = ActionManager::instance()->actionContainer(M_FILE);
    auto actionInit = [&](QAction *action, QString actionID, QKeySequence key, QString iconFileName){
        action->setIcon(QIcon::fromTheme(iconFileName));
        auto cmd = ActionManager::instance()->registerAction(action, actionID);
        cmd->setDefaultKeySequence(key);
        mFile->addAction(cmd, G_FILE_NEW);
    };

    d->newAction.reset(new QAction(MWMFA_NEW_FILE_OR_PROJECT, this));
    actionInit(d->newAction.get(),
               "File.New.FileOrProject",
               QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_O), QString());
    QObject::connect(d->newAction.get(), &QAction::triggered,
                     this, &TemplateManager::newWizard, Qt::DirectConnection);
}

void TemplateManager::newWizard()
{
    MainDialog *mainDlg = new MainDialog();

    if (utils::isWayland()) {
        mainDlg->setWindowFlag(Qt::WindowStaysOnTopHint, true);
    }

    mainDlg->exec();
}
