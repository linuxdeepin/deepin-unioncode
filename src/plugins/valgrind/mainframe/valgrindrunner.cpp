/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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
#include "valgrindrunner.h"
#include "valgrindbar.h"

#include "common/common.h"
#include "common/widget/outputpane.h"
#include "base/abstractaction.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/language/languageservice.h"

#include <QProcess>
#include <QDebug>

typedef FileOperation FO;
using namespace dpfservice;

class ValgrindRunnerPrivate
{
    friend class ValgrindRunner;

    QStringList ValgrindArgs;
    dpfservice::ProjectInfo projectInfo;
    QString activedProjectKitName;
    QString workingDir;
    QString currentFilePath;
    QString targetPath;
    QString xmlFilePath;

    QSharedPointer<QAction> memcheckAction;
    QSharedPointer<QAction> helgrindAction;
};

ValgrindRunner::ValgrindRunner(QObject *parent)
    : QObject(parent)
    , d(new ValgrindRunnerPrivate)
{

}

void ValgrindRunner::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;

    d->memcheckAction.reset(new QAction(MWMAA_VALGRIND_MEMCHECK));
    ActionManager::getInstance()->registerAction(d->memcheckAction.get(), "Analyze.ValgrindMemcheck",
                                                 d->memcheckAction->text(), QKeySequence());
    windowService->addAction(MWM_ANALYZE, new AbstractAction(d->memcheckAction.get()));

    d->helgrindAction.reset(new QAction(MWMAA_VALGRIND_HELGRIND));
    ActionManager::getInstance()->registerAction(d->helgrindAction.get(), "Analyze.ValgrindHelgrind",
                                                 d->helgrindAction->text(), QKeySequence());
    windowService->addAction(MWM_ANALYZE, new AbstractAction(d->helgrindAction.get()));

    QObject::connect(d->memcheckAction.get(), &QAction::triggered, [=]() {
        QtConcurrent::run([=]() {
            ValgrindRunner::instance()->runValgrind("memcheck");
        });
     });

    QObject::connect(d->helgrindAction.get(), &QAction::triggered, [=]() {
        QtConcurrent::run([=]() {
            ValgrindRunner::instance()->runValgrind("helgrind");
        });
    });

    setActionsStatus(d->activedProjectKitName);
}

void ValgrindRunner::runValgrind(const QString &type)
{
    runBuilding();
    setValgrindArgs(type);
    QProcess procValgrind;
    connect(&procValgrind, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [&]() {
        emit valgrindFinished(d->xmlFilePath, type);
    });

    connect(&procValgrind, &QProcess::readyReadStandardError, [&]() {
        procValgrind.setReadChannel(QProcess::StandardError);
        editor.switchContext(tr("&Application Output"));
        while (procValgrind.canReadLine()) {
            QString line = QString::fromUtf8(procValgrind.readLine());
            OutputPane::instance()->appendText(line, OutputPane::OutputFormat::StdErr);
        }
    });

    connect(&procValgrind, &QProcess::readyReadStandardOutput, [&]() {
        procValgrind.setReadChannel(QProcess::StandardError);
        editor.switchContext(tr("&Application Output"));
        while (procValgrind.canReadLine()) {
            QString line = QString::fromUtf8(procValgrind.readLine());
            OutputPane::instance()->appendText(line, OutputPane::OutputFormat::StdOut);
        }
    });

    procValgrind.start("valgrind", d->ValgrindArgs);
    emit clearValgrindBar(type);
    procValgrind.waitForFinished(-1);
}

ValgrindRunner *ValgrindRunner::instance()
{
    static ValgrindRunner ins;
    return &ins;
}

void ValgrindRunner::setValgrindArgs(const QString &type)
{
    QString storage = FO::checkCreateDir(FO::checkCreateDir(d->workingDir, ".unioncode"), "valgrind");

    if (MEMCHECK == type) {
        d->ValgrindArgs.clear();
        d->xmlFilePath = storage + QDir::separator() + "memcheck.xml";
        d->ValgrindArgs << "--leak-check=full" << "--xml=yes" << "--show-leak-kinds=definite"
                        << "--xml-file=" + d->xmlFilePath << d->targetPath;
    } else if (HELGRIND == type) {
        d->ValgrindArgs.clear();
        d->xmlFilePath = storage + QDir::separator() + "helgrind.xml";
        d->ValgrindArgs << "--tool=helgrind" << "--xml=yes" << "--xml-file=" + d->xmlFilePath << d->targetPath;
    }
}

void ValgrindRunner::setMemcheckArgs(QStringList &args)
{
    //TODO:add config arguments
    args << "--leak-check=full" << "--xml=yes" << "--show-leak-kinds=definite";
}

void ValgrindRunner::setHelgrindArgs(QStringList &args)
{
    //TODO:add config arguments
    args << "--tool=helgrind" << "--xml=yes";
}

void ValgrindRunner::setActionsStatus(const QString &kitName)
{
    if (kitName == "ninja" || kitName == "cmake") {
        d->memcheckAction.get()->setEnabled(true);
        d->helgrindAction.get()->setEnabled(true);
    } else {
        d->memcheckAction.get()->setEnabled(false);
        d->helgrindAction.get()->setEnabled(false);
    }
}

void ValgrindRunner::saveCurrentProjectInfo(const ProjectInfo &projectInfo)
{
    d->projectInfo = projectInfo;
    d->activedProjectKitName = d->projectInfo.kitName();
    setActionsStatus(d->activedProjectKitName);
}

void ValgrindRunner::removeProjectInfo()
{
    d->activedProjectKitName.clear();
}

void ValgrindRunner::saveCurrentFilePath(const QString &filePath)
{
    d->currentFilePath = filePath;
}

void ValgrindRunner::removeCurrentFilePath()
{
    d->currentFilePath.clear();
}

void ValgrindRunner::runBuilding()
{
    auto &ctx = dpfInstance.serviceContext();
    LanguageService *service = ctx.service<LanguageService>(LanguageService::name());
    if (service) {
        auto generator = service->create<LanguageGenerator>(d->activedProjectKitName);
        if (generator) {
            if (generator->isNeedBuild()) {
                generator->build(d->projectInfo.workspaceFolder());
            }
            RunCommandInfo args = generator->getRunArguments(d->projectInfo, d->currentFilePath);
            d->targetPath = args.program;
            d->workingDir = args.workingDir;
        }
    }
}
