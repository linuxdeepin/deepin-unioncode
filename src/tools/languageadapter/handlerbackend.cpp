#include "handlerbackend.h"
#include "route.h"
#include "protocol.h"

#include <iostream>
#include <QJsonObject>

HandlerBackend::HandlerBackend(const SettingInfo &info)
    : info (info)
{
    bind(new QProcess);
}

void HandlerBackend::bind(QProcess *qIODevice)
{
    Handler::bind(qIODevice);
    qIODevice->setProgram(info.program);
    qIODevice->setArguments(info.arguments);
    qIODevice->start();
    qIODevice->waitForStarted(3000);
    connect(qIODevice, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &HandlerBackend::doFinished, Qt::UniqueConnection);
}

void HandlerBackend::filterData(const QByteArray &array)
{
    static QByteArray cache;
    if (array.startsWith("Content-Length:")) {
        cache = array;
        int index = cache.indexOf("\r\n\r\n");
        if (index == -1) {
            return;
        } else {
            QByteArray head = cache.mid(0, index);
            QList<QByteArray> headList = head.split(':');
            int readCount = headList[1].toInt();
            QByteArray data = cache.mid(head.size() + 4, readCount);
            if (data.size() < readCount) {
                return;
            } else if (data.size() > readCount) {
                qCritical() << "process data error";
                abort();
            } else {
                response(data);
                cache.remove(0, index);
                cache.remove(0, readCount + 4);
                if (!cache.isEmpty())
                    filterData(cache);
            }
        }
    } else {
        cache += array;
        filterData(cache);
    }
}

void HandlerBackend::response(const QByteArray &array)
{
    QJsonParseError err;
    QJsonObject jsonObj = QJsonDocument::fromJson(array, &err).object();
    if (jsonObj.isEmpty()) {
        qCritical() << err.errorString() << qPrintable(array);
        abort();
    }

    qInfo() << "Response <- " << qPrintable(array);

    if (exitNotification(jsonObj)) {
        return;
    }
}

bool HandlerBackend::exitNotification(const QJsonObject &obj)
{
    if (obj.value("method").toString() == "exit") {
        Route::instance()->removeFreeBackend(this);
        auto frontend = Route::instance()->findFrontend(this);
        if (!frontend) {
            std::cerr << "Unknown error, find frontend is nullptr"
                      << std::endl;
        } else {
            // 手动发送此消息
            frontend->nowToWrite(protocol::setHeader(obj).toUtf8());
            Route::instance()->removeFreeFronted(frontend);
        }
        return true;
    }

    return false;
}

void HandlerBackend::doFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCritical() << "Backend finished "
                << "name: " << info.program << info.arguments
                << "exitCode: " << exitCode
                << "exitStatus： " << exitStatus;

    auto frontend = Route::instance()->findFrontend(this);
    Route::instance()->removeFreeBackend(this);
    Route::instance()->removeFreeFronted(frontend);
}
