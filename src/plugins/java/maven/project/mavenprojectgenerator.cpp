// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mavenprojectgenerator.h"
#include "mavenasynparse.h"
#include "maven/project/properties/mavenconfigpropertywidget.h"

#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"

#include "common/dialog/propertiesdialog.h"
#include "properties/mavenconfigutil.h"
#include "common/util/custompaths.h"
#include "common/util/environment.h"
#include "common/supportfile/dapconfig.h"
#include "services/project/projectservice.h"

#include <QtXml>
#include <QFileIconProvider>

using namespace mavenConfig;
using namespace dpfservice;
class MavenProjectGeneratorPrivate
{
    friend class MavenProjectGenerator;
    QStandardItem* configureRootItem {nullptr};
    QMenu *mavenMenu {nullptr};
    QHash<QStandardItem*, MavenAsynParse*> projectParses {};
};

MavenProjectGenerator::MavenProjectGenerator()
    : d(new MavenProjectGeneratorPrivate())
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (!builderService) {
        qCritical() << "Failed, not found service : builderService";
        abort();
    }
}

MavenProjectGenerator::~MavenProjectGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QStringList MavenProjectGenerator::supportLanguages()
{
    return {dpfservice::MWMFA_JAVA};
}

QStringList MavenProjectGenerator::supportFileNames()
{
    return {"pom.xml"};
}

QDialog *MavenProjectGenerator::configureWidget(const QString &language,
                                         const QString &workspace)
{
    // get config result.
    ProjectInfo projectInfo;
    projectInfo.setLanguage(language);
    projectInfo.setKitName(MavenProjectGenerator::toolKitName());
    projectInfo.setWorkspaceFolder(workspace);

    // refresh config.
    restoreRuntimeCfg(projectInfo);
    configure(projectInfo);

    return nullptr;
}

bool MavenProjectGenerator::configure(const dpfservice::ProjectInfo &info)
{
    dpfservice::ProjectGenerator::configure(info);

    auto root = createRootItem(info);
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && root) {
        projectService->addRootItem(root);
        projectService->expandedDepth(root, 1);
    }

    return true;
}

QStandardItem *MavenProjectGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    QStandardItem *rootItem = ProjectGenerator::createRootItem(info);
    dpfservice::ProjectInfo::set(rootItem, info);
    d->projectParses[rootItem] = new MavenAsynParse();
    QObject::connect(d->projectParses[rootItem], &MavenAsynParse::itemsModified,
                     this, &MavenProjectGenerator::itemModified);
    QMetaObject::invokeMethod(d->projectParses[rootItem], "parseProject",
                              Q_ARG(const dpfservice::ProjectInfo &, info));
    return rootItem;
}

void MavenProjectGenerator::removeRootItem(QStandardItem *root)
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

QMenu *MavenProjectGenerator::createItemMenu(const QStandardItem *item)
{
    if (item->parent())
        return nullptr;

    QMenu *menu = new QMenu();
    if (!d->mavenMenu) {
        d->mavenMenu = new QMenu("Maven", menu);
        QObject::connect(d->mavenMenu, &QMenu::destroyed,
                         [=](){
            d->mavenMenu = nullptr;
        });
    }

    menu->addMenu(d->mavenMenu);

    // asyn parse
    using namespace dpfservice;

    QStandardItem *itemTemp = const_cast<QStandardItem *>(item);
    if (!itemTemp)
        return d->mavenMenu;

    auto parse = d->projectParses[itemTemp];
    if (!parse)
        return d->mavenMenu;

    ProjectInfo info = ProjectInfo::get(item);
    if (info.isEmpty())
        return d->mavenMenu;

    // add menu generat call back
    QObject::connect(parse, &MavenAsynParse::parsedActions,
                     this, &MavenProjectGenerator::doAddMavenMeue,
                     Qt::UniqueConnection);
    // execute logic
    parse->parseActions(info);

    QAction *action = new QAction(tr("Properties"));
    menu->addAction(action);
    QObject::connect(action, &QAction::triggered, [=](){
        actionProperties(info, itemTemp);
    });

    return menu;
}

void MavenProjectGenerator::itemModified(const QList<QStandardItem *> &items)
{
    MavenAsynParse *parse = qobject_cast<MavenAsynParse*>(sender());
    if (parse) {
        auto root = d->projectParses.key(parse);
        emit itemChanged(root, items);
    }
}

void MavenProjectGenerator::doAddMavenMeue(const ProjectActionInfos &infos)
{
    if (d->mavenMenu) {
        for (auto actionInfo : infos) {
            QAction *action = new QAction(actionInfo.displyText, d->mavenMenu);
            ProjectMenuActionInfo::set(action, actionInfo);
            d->mavenMenu->addAction(action);
            QObject::connect(action, &QAction::triggered,
                             this, &MavenProjectGenerator::doActionTriggered,
                             Qt::UniqueConnection);
        }
    }
}

void MavenProjectGenerator::doActionTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        auto value = ProjectMenuActionInfo::get(action);

        auto &ctx = dpfInstance.serviceContext();
        auto builderService = ctx.service<dpfservice::BuilderService>(dpfservice::BuilderService::name());
        if (builderService) {
            BuildCommandInfo commandInfo;
            commandInfo.kitName = toolKitName();
            commandInfo.program = value.buildProgram;
            commandInfo.arguments = value.buildArguments;
            commandInfo.workingDir = QFileInfo(value.workingDirectory).path();
            builderService->runbuilderCommand({commandInfo}, false);
        }
    }
}

void MavenProjectGenerator::actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item)
{
    PropertiesDialog dlg;
    MavenConfigPropertyWidget *property = new MavenConfigPropertyWidget(info, item);
    dlg.insertPropertyPanel("Config", property);
    dlg.exec();
}

void MavenProjectGenerator::restoreRuntimeCfg(dpfservice::ProjectInfo &projectInfo)
{
    // get project config.
    ConfigureParam *cfgParams = ConfigUtil::instance()->getConfigureParamPointer();
    ConfigUtil::instance()->readConfig(ConfigUtil::instance()->getConfigPath(projectInfo.workspaceFolder()), *cfgParams);

    // get global config.
    QString arch = ProcessUtil::localPlatform();
    QString dapSupportFilePath = support_file::DapSupportConfig::globalPath();
    QString configHomePath = env::pkg::native::path();// CustomPaths::user(CustomPaths::Configures) + QDir::separator();
    support_file::JavaDapPluginConfig javaDapPluginConfig;
    bool ret = support_file::DapSupportConfig::readFromSupportFile(dapSupportFilePath, arch, javaDapPluginConfig, configHomePath);
    if (!ret) {
        qDebug("Read dapconfig.support failed, please check the file and retry.");
        return;
    }

    // use global config when project config is null.
    auto setPropertyParams = [&](QString &projectProperty, const QString &key, const QString &globalPropery){
        if (projectProperty.isEmpty()) {
            // use global propery.
            projectInfo.setProperty(key, globalPropery);
            projectProperty = globalPropery;
        } else {
            // use project propery.
            projectInfo.setProperty(key, projectProperty);
        }
    };
    setPropertyParams(cfgParams->jrePath, kJrePath, javaDapPluginConfig.jrePath);
    setPropertyParams(cfgParams->jreExecute, kJreExecute, javaDapPluginConfig.jreExecute);
    setPropertyParams(cfgParams->launchConfigPath, kLaunchConfigPath, javaDapPluginConfig.launchConfigPath);
    setPropertyParams(cfgParams->launchPackageFile, kLaunchPackageFile, javaDapPluginConfig.launchPackageFile);
    setPropertyParams(cfgParams->dapPackageFile, kDapPackageFile, javaDapPluginConfig.dapPackageFile);
    projectInfo.setDetailInformation(cfgParams->detailInfo);
}
