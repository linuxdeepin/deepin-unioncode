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
#include "ninjagenerator.h"
#include "ninjaasynparse.h"
#include "mainframe/properties/configpropertywidget.h"

#include "common/dialog/propertiesdialog.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/option/optionmanager.h"

#include <QtConcurrent>
#include <QtXml>
#include <QFileIconProvider>

class NinjaGeneratorPrivate
{
    friend class NinjaGenerator;
    QStandardItem* configureRootItem {nullptr};
    QMenu *ninjaMenu {nullptr};
    QProcess *menuGenProcess {nullptr};
    QHash<QStandardItem*, NinjaAsynParse*> projectParses {};
};

NinjaGenerator::NinjaGenerator()
    : d(new NinjaGeneratorPrivate())
{
    qRegisterMetaType<QList<QStandardItem*>>("QList<QStandardItem*>");
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService) {
        qCritical() << "Failed, not found service : projectService";
        abort();
    }
}

NinjaGenerator::~NinjaGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QStringList NinjaGenerator::supportLanguages()
{
    return {dpfservice::MWMFA_CXX};
}

QStringList NinjaGenerator::supportFileNames()
{
    return {"build.ninja"};
}

QDialog *NinjaGenerator::configureWidget(const QString &language,
                                          const QString &projectPath)
{
    using namespace dpfservice;

    ProjectInfo info;
    info.setLanguage(language);
    info.setKitName(NinjaGenerator::toolKitName());
    info.setWorkspaceFolder(projectPath);

    configure(info);

    return nullptr;
}

bool NinjaGenerator::configure(const dpfservice::ProjectInfo &info)
{
    dpfservice::ProjectGenerator::configure(info);

    auto root = createRootItem(info);
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && root) {
        projectService->projectView.addRootItem(root);
        projectService->projectView.expandedDepth(root, 1);
    }

    dpfservice::ProjectGenerator::configure(info);

    return true;
}

QStandardItem *NinjaGenerator::createRootItem(const dpfservice::ProjectInfo &info)
{
    using namespace dpfservice;
    QStandardItem * rootItem = ProjectGenerator::createRootItem(info);
    dpfservice::ProjectInfo::set(rootItem, info);
    d->projectParses[rootItem] = new NinjaAsynParse();
    QObject::connect(d->projectParses[rootItem], &NinjaAsynParse::itemsModified,
                     this, &NinjaGenerator::doProjectChildsModified);
    QMetaObject::invokeMethod(d->projectParses[rootItem], "parseProject",
                              Q_ARG(const dpfservice::ProjectInfo &, info));
    return rootItem;
}

void NinjaGenerator::removeRootItem(QStandardItem *root)
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

QMenu *NinjaGenerator::createItemMenu(const QStandardItem *item)
{
    Q_UNUSED(item)
    return nullptr;
}

void NinjaGenerator::doProjectChildsModified(const QList<QStandardItem *> &items)
{
    auto rootItem = d->projectParses.key(qobject_cast<NinjaAsynParse*>(sender()));
    if (rootItem) {
        while (rootItem->hasChildren()) {
            rootItem->takeRow(0);
        }
        rootItem->appendRows(items);
    }
}

void NinjaGenerator::doGradleGeneratMenu(const QString &program,
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
            if (d->menuGenProcess->canReadLine() && d->ninjaMenu) {
                QTextStream stream(d->menuGenProcess->readAll());
                while (!stream.atEnd()) {
                    QString taskBegin = stream.readLine();
                    QMenu *menu = nullptr;
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
                                action->setProperty("kitName", NinjaGenerator::toolKitName());
#ifdef __WIN32__
                                action->setPriority("program", "./gradlew.bat");
#elif __linux__ ||  __apple__
                                action->setProperty("program", OptionManager::getInstance()->getGradleToolPath());
#endif
                                action->setProperty("arguments", QStringList({"task", taskChild[0]}));
                                action->setProperty("workDir", d->menuGenProcess->workingDirectory());
                                QObject::connect(action, &QAction::triggered, this,
                                                 &NinjaGenerator::doGradleTaskActionTriggered,
                                                 Qt::UniqueConnection);
                                menu->addAction(action);
                            }
                        }
                        d->ninjaMenu->addMenu(menu);
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

void NinjaGenerator::doGradleCleanMenu()
{
    if (d->ninjaMenu) {
        for (auto &action : d->ninjaMenu->actions()) {
            d->ninjaMenu->removeAction(action);
        }
    }
}

void NinjaGenerator::doGradleTaskActionTriggered()
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
            builderService->interface.builderCommand(commandInfo);
        }
    }
}

void NinjaGenerator::actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item)
{
    PropertiesDialog dlg;
    ConfigPropertyWidget *property = new ConfigPropertyWidget(info, item);
    dlg.insertPropertyPanel("Config", property);
    dlg.exec();
}
