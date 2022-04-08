#include "workspacecmake.h"
#include "processdialog.h"
#include "common/common.h"
#include "workspacedata.h"
#include <QProcess>

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
    QFileInfo fileInfo(CustomPaths::endSeparator(targetPath) += d->compile_commands);
    if (fileInfo.exists()) {
        WorkspaceData::globalInstance()->addWorkspace(rootPath);
        return result;
    }

    ProcessDialog dialog;
    d->process->setWorkingDirectory(targetPath);
    d->process->setProgram("cmake");
    d->process->setArguments({rootPath, "-DCMAKE_EXPORT_COMPILE_COMMANDS=1"});
    d->process->start();

    QString message = QString("command:") + ">> cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1";
    dialog << message;

    QObject::connect(d->process, &QProcess::readyRead, [=, &dialog, &message](){
        message += d->process->readAll();
        dialog << message;
    });

    QObject::connect(d->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [=, &dialog, &result](int exitCode, QProcess::ExitStatus exitStatus)
    {
        QTimer::singleShot(2000, [&dialog, &result, &exitStatus](){
            if (QProcess::ExitStatus::NormalExit == exitStatus)
                result = true;
            dialog.setRunning(false);
            dialog.close();
        });
    });
    dialog.setTitle("cmake generate workspace");
    dialog.setRunning(true);
    dialog.exec();

    return result;
}
