// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pythonprojectgenerator.h"
#include "pythonasynparse.h"
#include "python/project/properties/configpropertywidget.h"
#include "python/project/properties/configutil.h"
#include "common/dialog/propertiesdialog.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

#include <QtConcurrent>
#include <QtXml>
#include <QFileIconProvider>

using namespace config;
class PythonProjectGeneratorPrivate
{
    friend class PythonProjectGenerator;
    QStandardItem* configureRootItem {nullptr};
    QMenu *pythonMenu {nullptr};
    QProcess *menuGenProcess {nullptr};
    QHash<QStandardItem*, PythonAsynParse*> projectParses {};
    dpfservice::ProjectInfo prjInfo;
};

PythonProjectGenerator::PythonProjectGenerator()
    : d(new PythonProjectGeneratorPrivate())
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService) {
        qCritical() << "Failed, not found service : projectService";
        abort();
    }
}

PythonProjectGenerator::~PythonProjectGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QStringList PythonProjectGenerator::supportLanguages()
{
    return {dpfservice::MWMFA_PYTHON};
}

DWidget *PythonProjectGenerator::configureWidget(const QString &language,
                                          const QString &projectPath)
{
    using namespace dpfservice;

    d->prjInfo.setLanguage(language);
    d->prjInfo.setKitName(PythonProjectGenerator::toolKitName());
    d->prjInfo.setWorkspaceFolder(projectPath);
    d->prjInfo.setExePrograms({exeCurrent, exeEntry});

    return nullptr;
}

void PythonProjectGenerator::acceptConfigure()
{
    configure(d->prjInfo);
}

bool PythonProjectGenerator::configure(const dpfservice::ProjectInfo &projectInfo)
{
    dpfservice::ProjectGenerator::configure(projectInfo);

    auto root = createRootItem(projectInfo);
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && root) {
        projectService->addRootItem(root);
        projectService->expandedDepth(root, 1);
    }
    ProjectConfigure *param = ConfigUtil::instance()->getConfigureParamPointer();
    ConfigUtil::instance()->readConfig(ConfigUtil::instance()->getConfigPath(projectInfo.workspaceFolder()), *param);

    return true;
}

QStandardItem *PythonProjectGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;

    QStandardItem * rootItem = ProjectGenerator::createRootItem(info);
    d->projectParses[rootItem] = new PythonAsynParse();
    QObject::connect(d->projectParses[rootItem],
                     &PythonAsynParse::itemsModified,
                     this, &PythonProjectGenerator::doProjectChildsModified,
                     Qt::ConnectionType::UniqueConnection);
    d->projectParses[rootItem]->parseProject(info);
    auto sourceFiles = d->projectParses[rootItem]->getFilelist();
    dpfservice::ProjectInfo tempInfo = info;
    tempInfo.setSourceFiles(sourceFiles);
    dpfservice::ProjectInfo::set(rootItem, tempInfo);

    return rootItem;
}

void PythonProjectGenerator::removeRootItem(QStandardItem *root)
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

QMenu *PythonProjectGenerator::createItemMenu(const QStandardItem *item)
{
    if (item->parent())
        return nullptr;

    QMenu *menu = new QMenu();
    dpfservice::ProjectInfo info = dpfservice::ProjectInfo::get(item);
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

void PythonProjectGenerator::doProjectChildsModified(const QList<QStandardItem *> &info)
{
    auto rootItem = d->projectParses.key(qobject_cast<PythonAsynParse*>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->takeRow(0);
        }
        rootItem->appendRows(info);
    }

    rootItem->setData(ParsingState::Done, Parsing_State_Role);
}

void PythonProjectGenerator::doPythonCleanMenu()
{
    if (d->pythonMenu) {
        for (auto &action : d->pythonMenu->actions()) {
            d->pythonMenu->removeAction(action);
        }
    }
}

void PythonProjectGenerator::actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item)
{
    PropertiesDialog dlg;
    ConfigPropertyWidget *property = new ConfigPropertyWidget(info, item);
    dlg.insertPropertyPanel(tr("Config"), property);
    dlg.exec();
}
