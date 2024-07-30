// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gradleprojectgenerator.h"
#include "transceiver/projectgradlereciver.h"
#include "gradleasynparse.h"
#include "properties/gradleconfigpropertywidget.h"

#include "common/dialog/propertiesdialog.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"
#include "properties/gradleconfigutil.h"
#include "common/util/custompaths.h"
#include "common/util/environment.h"
#include "common/supportfile/dapconfig.h"
#include "services/project/projectservice.h"

#include <QtConcurrent>
#include <QtXml>
#include <QFileIconProvider>

using namespace gradleConfig;
using namespace dpfservice;
enum_def(GradleShellKey, QString)
{
    enum_exp ScriptName = "gradlew";
    enum_exp ScriptArg_Task = "tasks";
};

enum_def(GradleMenuKey, QString)
{
    enum_exp Build_Tasks = "Build tasks";
    enum_exp Build_Setup_Tasks = "Build Setup tasks";
    enum_exp Focumentation_Tasks = "Documentation tasks";
    enum_exp Help_Tasks = "Help tasks";
    enum_exp Verification_tasks = "Verification tasks";
};

struct GradleTasksHelp
{
    QString argsName;
    QString argsHelp;
};

typedef QList<GradleTasksHelp> GradleTasksHelps;
typedef QHash<QString, GradleTasksHelps> GradleTasks;

class GradleProjectGeneratorPrivate
{
    friend class GradleProjectGenerator;
    QStandardItem* configureRootItem {nullptr};
    QMenu *gradleMenu {nullptr};
    QProcess *menuGenProcess {nullptr};
    QHash<QStandardItem*, GradleAsynParse*> projectParses {};
    QSet<QString> toExpand;
    ProjectInfo prjInfo;
private:

    GradleTasks parseTasks(const QByteArray &data)
    {
        GradleTasks result;
        QTextStream textStream(data);
        while (!textStream.atEnd()) {
            qInfo() << textStream.readLine();
        }
        return result;
    }

    GradleTasks getTasks(QString sourceFolder,
                         QString shellName = GradleShellKey::get()->ScriptName)
    {
        GradleTasks result;
        QString program = sourceFolder + QDir::separator() +shellName;
        QStringList args = {GradleShellKey::get()->ScriptArg_Task};
        ProcessUtil::execute(program, args, sourceFolder, [&](const QByteArray &data){
            result = parseTasks(data);
        });
        return result;
    }
};

GradleProjectGenerator::GradleProjectGenerator()
    : d(new GradleProjectGeneratorPrivate())
{
    qRegisterMetaType<QList<QStandardItem*>>("QList<QStandardItem*>");
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService) {
        qCritical() << "Failed, not found service : projectService";
        abort();
    }
    QObject::connect(ProjectGradleProxy::instance(),
                     &ProjectGradleProxy::nodeExpanded,
                     this, [this](const QString &filePath){
        d->toExpand.insert(filePath);
    });

    QObject::connect(ProjectGradleProxy::instance(),
                     &ProjectGradleProxy::nodeCollapsed,
                     this, [this](const QString &filePath){
        d->toExpand.remove(filePath);
    });
}

GradleProjectGenerator::~GradleProjectGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QStringList GradleProjectGenerator::supportLanguages()
{
    return {dpfservice::MWMFA_JAVA};
}

QStringList GradleProjectGenerator::supportFileNames()
{
    return {"build.gradle", "settings.gradle"};
}

DWidget *GradleProjectGenerator::configureWidget(const QString &language,
                                          const QString &projectPath)
{
    // get config result.
    d->prjInfo.setLanguage(language);
    d->prjInfo.setKitName(GradleProjectGenerator::toolKitName());
    d->prjInfo.setWorkspaceFolder(projectPath);

    return nullptr;
}

void GradleProjectGenerator::acceptConfigure()
{
    restoreRuntimeCfg(d->prjInfo);
    configure(d->prjInfo);
}

bool GradleProjectGenerator::configure(const dpfservice::ProjectInfo &info)
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

    dpfservice::ProjectGenerator::configure(info);

    return true;
}

QStandardItem *GradleProjectGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    QStandardItem * rootItem = ProjectGenerator::createRootItem(info);
    dpfservice::ProjectInfo::set(rootItem, info);
    d->projectParses[rootItem] = new GradleAsynParse();

    QObject::connect(d->projectParses[rootItem], &GradleAsynParse::itemsModified, this, [=](const QList<QStandardItem *> &items){
        doProjectChildsModified(items);
        auto prjService = dpfGetService(ProjectService);
        prjService->expandItemByFile(d->toExpand.toList());
    });
    QMetaObject::invokeMethod(d->projectParses[rootItem], "parseProject",
                              Q_ARG(const dpfservice::ProjectInfo &, info));

    return rootItem;
}

void GradleProjectGenerator::removeRootItem(QStandardItem *root)
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

QMenu *GradleProjectGenerator::createItemMenu(const QStandardItem *item)
{
    if (item->parent())
        return nullptr;

    using namespace dpfservice;
    ProjectInfo info = ProjectInfo::get(item);
    if (!QFileInfo(info.workspaceFolder()).exists())
        return nullptr;

    QString program = info.workspaceFolder() + QDir::separator()
            + GradleShellKey::get()->ScriptName;
    QStringList args = {GradleShellKey::get()->ScriptArg_Task};
    QMenu *menu = new QMenu();
    if (!d->gradleMenu) {
        d->gradleMenu = new QMenu("Gradle");
        doGradleGeneratMenu(program, args, info.workspaceFolder()); // asyn
    }
    menu->addMenu(d->gradleMenu);

    QStandardItem *itemTemp = const_cast<QStandardItem *>(item);
    if (!itemTemp)
        return d->gradleMenu;

    QAction *action = new QAction(tr("Properties"));
    menu->addAction(action);
    QObject::connect(action, &QAction::triggered, [=](){
        actionProperties(info, itemTemp);
    });

    return menu;
}

void GradleProjectGenerator::doProjectChildsModified(const QList<QStandardItem *> &items)
{
    auto rootItem = d->projectParses.key(qobject_cast<GradleAsynParse*>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->takeRow(0);
        }
        rootItem->appendRows(items);
    }
    rootItem->setData(ParsingState::Done, Parsing_State_Role);
}

void GradleProjectGenerator::doGradleGeneratMenu(const QString &program,
                                          const QStringList &args,
                                          const QString &workdir)
{
    struct {
        QString operator ()(int count)
        {
            QString result;
            for(int i= 0; i < count; i++) {
                result += "-";
            }
            return result;
        }
    }taskChildSplit;

    if (!d->menuGenProcess) {
        d->menuGenProcess = new QProcess();

        QObject::connect(d->menuGenProcess, &QProcess::errorOccurred,
                         [=](QProcess::ProcessError error){
            qCritical() << "program: " << d->menuGenProcess->program()
                        << "aruments: " << d->menuGenProcess->arguments()
                        << "workdir: " << d->menuGenProcess->workingDirectory()
                        << "error" << error
                        << "errorString" << d->menuGenProcess->errorString();
        });

        QObject::connect(d->menuGenProcess,
                         QOverload<int, QProcess::ExitStatus>
                         ::of(&QProcess::finished),
                         [&](int, QProcess::ExitStatus)
        {
            if (d->menuGenProcess->canReadLine() && d->gradleMenu) {
                QTextStream stream(d->menuGenProcess->readAll());
                while (!stream.atEnd()) {
                    QString taskBegin = stream.readLine();
                    QMenu *menu = nullptr;
                    if (GradleMenuKey::contains(taskBegin)) {
                        menu = new QMenu(taskBegin);
                        menu->setToolTipsVisible(true);
                    }

                    if (menu) {
                        QString taskEnd = stream.readLine();
                        while (taskEnd != "") {
                            taskEnd = stream.readLine();
                            if (taskChildSplit(taskBegin.size())
                                    == taskEnd) {
                                continue;
                            }
                            QStringList taskChild = taskEnd.split(" - ");
                            if (taskChild.size() == 2) {
                                QAction *action = new QAction(taskChild[0]);
                                qInfo() << taskChild[0] << taskChild[1];
                                action->setToolTip(taskChild[1]);
                                action->setProperty("kitName", GradleProjectGenerator::toolKitName());
#ifdef __WIN32__
                                action->setPriority("program", "./gradlew.bat");
#elif __linux__ ||  __apple__
                                action->setProperty("program", OptionManager::getInstance()->getGradleToolPath());
#endif
                                action->setProperty("arguments", QStringList({"task", taskChild[0]}));
                                action->setProperty("workDir", d->menuGenProcess->workingDirectory());
                                QObject::connect(action, &QAction::triggered, this,
                                                 &GradleProjectGenerator::doGradleTaskActionTriggered,
                                                 Qt::UniqueConnection);
                                menu->addAction(action);
                            }
                        }
                        d->gradleMenu->addMenu(menu);
                    } // menu is create
                } // while readline
            } // if can read line
        });
        d->menuGenProcess->setProgram(program);
        d->menuGenProcess->setArguments(args);
        d->menuGenProcess->setWorkingDirectory(workdir);
        d->menuGenProcess->start();
    }
}

void GradleProjectGenerator::doGradleCleanMenu()
{
    if (d->gradleMenu) {
        for (auto &action : d->gradleMenu->actions()) {
            d->gradleMenu->removeAction(action);
        }
    }
}

void GradleProjectGenerator::doGradleTaskActionTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        auto &ctx = dpfInstance.serviceContext();
        auto builderService = ctx.service<dpfservice::BuilderService>(dpfservice::BuilderService::name());
        if (builderService) {
            BuildCommandInfo commandInfo;
            commandInfo.kitName = action->property("kitName").toString();
            commandInfo.program = action->property("program").toString();
            commandInfo.arguments = action->property("arguments").toStringList();
            commandInfo.workingDir = action->property("workDir").toString();
            builderService->runbuilderCommand({commandInfo}, false);
        }
    }
}

void GradleProjectGenerator::actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item)
{
    PropertiesDialog dlg;
    dlg.setCurrentTitle(info.currentProgram() + " - " + tr("Project Properties"));
    GradleConfigPropertyWidget *property = new GradleConfigPropertyWidget(info, item);
    dlg.insertPropertyPanel("Config", property);
    dlg.exec();
}

void GradleProjectGenerator::restoreRuntimeCfg(dpfservice::ProjectInfo &projectInfo)
{
    // get project config.
    ConfigureParam *cfgParams = ConfigUtil::instance()->getConfigureParamPointer();
    ConfigUtil::instance()->readConfig(ConfigUtil::instance()->getConfigPath(projectInfo.workspaceFolder()), *cfgParams);

    // get global config.
    QString arch = ProcessUtil::localPlatform();
    QString dapSupportFilePath = support_file::DapSupportConfig::globalPath();
    QString configHomePath = env::pkg::native::path();//CustomPaths::user(CustomPaths::Configures) + QDir::separator();
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
