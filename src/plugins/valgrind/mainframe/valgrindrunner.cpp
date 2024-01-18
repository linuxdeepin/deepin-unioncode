// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "valgrindrunner.h"
#include "valgrindbar.h"

#include "common/common.h"
#include "base/abstractaction.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"
#include "services/language/languageservice.h"

#include <QProcess>
#include <QDebug>
#include <QTextBlock>

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

    d->memcheckAction.reset(new QAction(MWMTA_VALGRIND_MEMCHECK, this));
    ActionManager::getInstance()->registerAction(d->memcheckAction.get(), "Analyze.ValgrindMemcheck",
                                                 d->memcheckAction->text(), QKeySequence());
    windowService->addAction(MWM_TOOLS, new AbstractAction(d->memcheckAction.get()));

    d->helgrindAction.reset(new QAction(MWMTA_VALGRIND_HELGRIND, this));
    ActionManager::getInstance()->registerAction(d->helgrindAction.get(), "Analyze.ValgrindHelgrind",
                                                 d->helgrindAction->text(), QKeySequence());
    windowService->addAction(MWM_TOOLS, new AbstractAction(d->helgrindAction.get()));

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
    if(!checkValgrindToolPath())
        return;
    runBuilding();
    setValgrindArgs(type);
    QProcess procValgrind;
    connect(&procValgrind, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [&]() {
        emit valgrindFinished(d->xmlFilePath, type);
    });

    connect(&procValgrind, &QProcess::readyReadStandardError, [&]() {
        procValgrind.setReadChannel(QProcess::StandardError);
        while (procValgrind.canReadLine()) {
            QString line = QString::fromUtf8(procValgrind.readLine());
            outputMsg(line, OutputPane::OutputFormat::StdErr);
        }
    });

    connect(&procValgrind, &QProcess::readyReadStandardOutput, [&]() {
        procValgrind.setReadChannel(QProcess::StandardError);
        while (procValgrind.canReadLine()) {
            QString line = QString::fromUtf8(procValgrind.readLine());
            outputMsg(line, OutputPane::OutputFormat::StdOut);
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
    setActionsStatus("");
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
            d->targetPath = args.program.trimmed();
            d->workingDir = args.workingDir.trimmed();
        }
    }
}

void ValgrindRunner::outputMsg(const QString &content, OutputPane::OutputFormat format)
{
    QMetaObject::invokeMethod(this, "printOutput", Q_ARG(QString, content), Q_ARG(OutputPane::OutputFormat, format));
}

bool ValgrindRunner::checkValgrindToolPath()
{
    if (!QFile("/usr/bin/valgrind").exists()) {
        QString retMsg = tr("please install valgrind tool in console with \"sudo apt install valgrind\".");
        outputMsg(retMsg, OutputPane::OutputFormat::StdErr);
        return false;
    }
    return true;
}

void ValgrindRunner::printOutput(const QString &content, OutputPane::OutputFormat format)
{
    uiController.switchContext(tr("&Application Output"));
    auto outputPane = OutputPane::instance();
    QString outputContent = content;
    if (format == OutputPane::OutputFormat::NormalMessage) {
        QTextDocument *doc = outputPane->document();
        QTextBlock tb = doc->lastBlock();
        QString lastLineText = tb.text();
        QString prefix = "\n";
        if (lastLineText.isEmpty()) {
            prefix = "";
        }
        QDateTime curDatetime = QDateTime::currentDateTime();
        QString time = curDatetime.toString("hh:mm:ss");
        outputContent = prefix + time + ":" + content;
    }
    outputContent += "\n";
    OutputPane::AppendMode mode = OutputPane::AppendMode::Normal;
    outputPane->appendText(outputContent, format, mode);
}
