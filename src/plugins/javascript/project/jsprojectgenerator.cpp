// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jsprojectgenerator.h"
#include "properties/configpropertywidget.h"
#include "common/dialog/propertiesdialog.h"
#include "services/window/windowservice.h"

#include <QtConcurrent>
#include <QFileIconProvider>

using namespace dpfservice;
class JSProjectGeneratorPrivate
{
    friend class JSProjectGenerator;
    QMenu *jsMenu {nullptr};
    ProjectInfo prjInfo;
};

JSProjectGenerator::JSProjectGenerator()
    : d(new JSProjectGeneratorPrivate())
{
    ProjectService *projectService = dpfGetService(ProjectService);
    if (!projectService) {
        qCritical() << "Failed, not found service : projectService";
        abort();
    }
}

JSProjectGenerator::~JSProjectGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QStringList JSProjectGenerator::supportLanguages()
{
    return {"JS"};
}

QMenu *JSProjectGenerator::createItemMenu(const QStandardItem *item)
{
    if (item->parent())
        return nullptr;

    QMenu *menu = new QMenu();
    ProjectInfo info = ProjectInfo::get(item);
    if (info.isEmpty())
        return nullptr;

    QStandardItem *itemTemp = const_cast<QStandardItem *>(item);
    if (!itemTemp)
        return nullptr;

    QAction *action = new QAction(tr("Properties"));
    menu->addAction(action);
    QObject::connect(action, &QAction::triggered, [=](){
        actionProperties(info, itemTemp);
    });

    return menu;
}

void JSProjectGenerator::doJSCleanMenu()
{
    if (d->jsMenu) {
        for (auto &action : d->jsMenu->actions()) {
            d->jsMenu->removeAction(action);
        }
    }
}

void JSProjectGenerator::actionProperties(const ProjectInfo &info, QStandardItem *item)
{
    PropertiesDialog dlg;
    ConfigPropertyWidget *property = new ConfigPropertyWidget(info, item);
    dlg.insertPropertyPanel("Config", property);
    dlg.exec();
}
