// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jsprojectgenerator.h"
#include "jsasynparse.h"
#include "properties/configpropertywidget.h"
#include "common/dialog/propertiesdialog.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

#include <QtConcurrent>
#include <QtXml>
#include <QFileIconProvider>

using namespace dpfservice;
class JSProjectGeneratorPrivate
{
    friend class JSProjectGenerator;
    QStandardItem* configureRootItem {nullptr};
    QMenu *jsMenu {nullptr};
    QProcess *menuGenProcess {nullptr};
    QHash<QStandardItem*, JSAsynParse*> projectParses {};
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

QDialog *JSProjectGenerator::configureWidget(const QString &language,
                                          const QString &projectPath)
{
    ProjectInfo info;
    info.setLanguage(language);
    info.setKitName(JSProjectGenerator::toolKitName());
    info.setWorkspaceFolder(projectPath);

    configure(info);

    return nullptr;
}

bool JSProjectGenerator::configure(const ProjectInfo &info)
{
    ProjectGenerator::configure(info);

    auto root = createRootItem(info);
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && root) {
        projectService->projectView.addRootItem(root);
        projectService->projectView.expandedDepth(root, 1);
    }

    return true;
}

QStandardItem *JSProjectGenerator::createRootItem(const ProjectInfo &info)
{
    QStandardItem * rootItem = ProjectGenerator::createRootItem(info);

    d->projectParses[rootItem] = new JSAsynParse();
    QObject::connect(d->projectParses[rootItem],
                     &JSAsynParse::itemsModified,
                     this, &JSProjectGenerator::doProjectChildsModified,
                     Qt::ConnectionType::UniqueConnection);

    ProjectInfo tempInfo = info;
    d->projectParses[rootItem]->parseProject(tempInfo);
    ProjectInfo::set(rootItem, tempInfo);

    return rootItem;
}

void JSProjectGenerator::removeRootItem(QStandardItem *root)
{
    if (!root)
        return;
    auto parser = d->projectParses[root];

    while (root->hasChildren()) {
        root->takeRow(0);
    }
    d->projectParses.remove(root);

    delete root;

    if (parser)
        delete parser;
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

void JSProjectGenerator::doProjectChildsModified(const QList<QStandardItem *> &info)
{
    auto rootItem = d->projectParses.key(qobject_cast<JSAsynParse*>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->takeRow(0);
        }
        rootItem->appendRows(info);
    }
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
