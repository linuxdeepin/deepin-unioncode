#include "workspacecmake.h"
#include "services/window/windowservice.h"
#include "common/common.h"
#include "workspacedata.h"
#include <QProcess>
using namespace dpfservice;
class WorkspaceCMakePrivate
{
    friend class WorkspaceCMake;
    QProcess *process = nullptr;
    QString compile_commands = "compile_commands.json";
};

WorkspaceCMake::WorkspaceCMake()
    : d(new WorkspaceCMakePrivate)
{
    d->process = new QProcess;
}

WorkspaceCMake::~WorkspaceCMake(){
    if (d) {
        delete d;
    }
}

bool WorkspaceCMake::generate(const QString &rootPath, const QString &targetPath)
{
    bool result = false;
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    windowService->showMessageDisplay();

    QFileInfo fileInfo(CustomPaths::endSeparator(targetPath) += d->compile_commands);
    if (fileInfo.exists()) {
        WorkspaceData::globalInstance()->addWorkspace(rootPath);
        return result;
    }

    d->process->setWorkingDirectory(targetPath);
    d->process->setProgram("cmake");
    d->process->setArguments({rootPath, "-DCMAKE_EXPORT_COMPILE_COMMANDS=1"});
    d->process->start();

    QString message = QString("command:") + ">> cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1";

    static int currentNumber = 10;
    windowService->appendProcessMessage(message, currentNumber);

    QObject::connect(d->process, &QProcess::readyRead, [=, &windowService](){
        currentNumber += 10;
        if (currentNumber >= 100) {
            windowService->appendProcessMessage(d->process->readAll(), currentNumber, currentNumber + 10);
        } else {
            windowService->appendProcessMessage(d->process->readAll(), currentNumber);
        }
    });

    QObject::connect(d->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [=, &windowService, &result](int exitCode, QProcess::ExitStatus exitStatus)
    {
        windowService->hideMessageDisplay();
    });

    return result;
}
