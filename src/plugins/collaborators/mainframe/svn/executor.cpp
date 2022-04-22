#include "executor.h"

SvnGenerator::SvnGenerator(bool autoDelete)
    : autoDeleteFlag(autoDelete)
{
    QObject::connect(&process, QOverload<int>::of(&QProcess::finished),
                     [=](int exitCode){
        emit finished(exitCode);
        QTimer::singleShot(100, [=]() { delete this;});
    });

    QObject::connect(&process, &QProcess::readyRead, [=](){
        emit message(process.readAll());
    });
}

void SvnGenerator::start(const QString &cmd, const QStringList &args)
{
    process.setProgram(cmd);
    process.setArguments(args);
    emit started();
    process.startDetached();
}
