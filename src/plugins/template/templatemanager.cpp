/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
        windowService->addAction(dpfservice::MWM_FILE, actionImpl);
    };

    d->newAction.reset(new QAction("New File or Project..."));
    actionInit(d->newAction.get(),
               "Fiel.New.FileOrProject",
               QKeySequence(Qt::Modifier::CTRL | Qt::Modifier::SHIFT | Qt::Key::Key_O), "new_doc.png");
    QObject::connect(d->newAction.get(), &QAction::triggered,
                     this, &TemplateManager::newWizard, Qt::DirectConnection);
}

void TemplateManager::newWizard()
{
    MainDialog *mainDlg = new MainDialog();
    mainDlg->exec();
}
