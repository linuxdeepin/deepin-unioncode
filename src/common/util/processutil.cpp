#include "processutil.h"

#include <QProcess>
#include <QFileInfo>
#include <QDebug>

bool ProcessUtil::execute(const QString &program,
                          const QStringList &arguments,
                          ProcessUtil::ReadCallBack func)
{
    bool ret = false;
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.connect(&process, QOverload<int, QProcess::ExitStatus >::of(&QProcess::finished),
                    [&ret, &process](int exitCode, QProcess::ExitStatus exitStatus){
        if (exitCode == 0 && exitStatus == QProcess::NormalExit)
            ret = true;
        qInfo() << process.program() << process.arguments();
    });

    if (func) {
        QProcess::connect(&process, &QProcess::readyRead, [&process, &func](){
            func(process.readAll());
        });
    }
    process.start();
    process.waitForFinished();

    return ret;
}

bool ProcessUtil::exists(const QString &name)
{
    bool ret = false;
#ifdef linux
    auto outCallback = [&ret, &name](const QByteArray &array) {
        QList<QByteArray> rmSearch = array.split(' ');
        foreach (QByteArray rmProcess, rmSearch) {
            QFileInfo info(rmProcess);
            if (info.fileName() == name && info.isExecutable()) {
                if (!ret)
                    ret = true;
            }
        }
    };
    ret = ProcessUtil::execute("whereis", {name}, outCallback);
#else
#endif
    return ret;
}

bool ProcessUtil::hasGio()
{
    return exists("gio");
}

bool ProcessUtil::moveToTrash(const QString &filePath)
{
#ifdef linux
    if (!hasGio())
        return false;
    return ProcessUtil::execute("gio", {"trash", filePath});
#else

#endif
}
