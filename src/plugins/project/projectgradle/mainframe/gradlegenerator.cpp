/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "gradlegenerator.h"
#include "gradleasynparse.h"
#include "gradleitemkeeper.h"
#include "transceiver/sendevents.h"
#include "transceiver/projectgradlereceiver.h"
#include "services/window/windowservice.h"

#include <QtConcurrent>
#include <QtXml>
#include <QFileIconProvider>

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

class GradleGeneratorPrivate
{
    friend class GradleGenerator;
    QStandardItem* configureRootItem {nullptr};
    QMenu *gradleMenu {nullptr};
    QProcess *menuGenProcess {nullptr};
    QHash<QStandardItem*, GradleAsynParse*> projectParses {};
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

GradleGenerator::GradleGenerator()
    : d(new GradleGeneratorPrivate())
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService) {
        qCritical() << "Failed, not found service : projectService";
        abort();
    }
}

GradleGenerator::~GradleGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QWidget *GradleGenerator::configureWidget(const QString &language,
                                          const QString &projectPath)
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService)
        return nullptr;

    auto proInfos = projectService->projectView.getAllProjectInfo();
    for (auto &val : proInfos) {
        if (val.language() == language && projectPath == val.projectFilePath()) {
            ContextDialog::ok(QDialog::tr("Cannot open repeatedly!\n"
                                          "language : %0\n"
                                          "projectPath : %1")
                              .arg(language, projectPath));
            return nullptr;
        }
    }

    ProjectInfo info;
    info.setLanguage(language);
    info.setSourceFolder(projectPath);
    info.setKitName(GradleGenerator::toolKitName());
    info.setWorkspaceFolder(projectPath);
    info.setProjectFilePath(projectPath);

    configure(info);

    return nullptr;
}

bool GradleGenerator::configure(const dpfservice::ProjectInfo &info)
{
    auto root = createRootItem(info);
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (projectService && windowService && root) {
        projectService->projectView.addRootItem(root);
        projectService->projectView.expandedDepth(root, 1);
        windowService->switchWidgetNavigation(MWNA_EDIT);
        windowService->switchWidgetWorkspace(MWCWT_PROJECTS);
    }

    Generator::started(); // emit starded
    return true;
}

QStandardItem *GradleGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;

    QStandardItem * rootItem = new QStandardItem(QFileInfo(info.sourceFolder()).fileName());
    dpfservice::ProjectInfo::set(rootItem, info);
    d->projectParses[rootItem] = new GradleAsynParse();
    QObject::connect(d->projectParses[rootItem],
                     &GradleAsynParse::itemsModified,
                     this, &GradleGenerator::doProjectChildsModified,
                     Qt::UniqueConnection);
    d->projectParses[rootItem]->parseProject(info);

    return rootItem;
}

void GradleGenerator::removeRootItem(QStandardItem *root)
{
    if (!root)
        return;

    auto parse = d->projectParses[root];
    if (parse)
        parse->removeRows();
    delete root;
    d->projectParses.remove(root);
}

QMenu *GradleGenerator::createItemMenu(const QStandardItem *item)
{
    if (item->parent())
        return nullptr;

    using namespace dpfservice;
    ProjectInfo info = ProjectInfo::get(item);
    if (!QFileInfo(info.sourceFolder()).exists())
        return nullptr;

    QString sourceFolder = info.sourceFolder();
    QString program = sourceFolder + QDir::separator()
            + GradleShellKey::get()->ScriptName;
    QStringList args = {GradleShellKey::get()->ScriptArg_Task};
    QMenu *menu = new QMenu();
    if (!d->gradleMenu) {
        d->gradleMenu = new QMenu("Gradle");
        doGradleGeneratMenu(program, args, sourceFolder); // asyn
    }
    menu->addMenu(d->gradleMenu);

    return menu;
}

void GradleGenerator::doProjectChildsModified(const dpfservice::ParseInfo<QList<QStandardItem *> > &info)
{
    auto rootItem = d->projectParses.key(qobject_cast<GradleAsynParse*>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->takeRow(0);
        }
        rootItem->appendRows(info.result);
    }
}

void GradleGenerator::doGradleGeneratMenu(const QString &program,
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
                                qInfo() << taskChild[1];
                                action->setToolTip(taskChild[1]);
                                QObject::connect(action, &QAction::triggered, this,
                                                 &GradleGenerator::doGradleTaskActionTriggered,
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

void GradleGenerator::doGradleCleanMenu()
{
    if (d->gradleMenu) {
        for (auto &action : d->gradleMenu->actions()) {
            d->gradleMenu->removeAction(action);
        }
    }
}

void GradleGenerator::doGradleTaskActionTriggered()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action->text().isEmpty()){
        // befor do execute gradle command
    }
}
