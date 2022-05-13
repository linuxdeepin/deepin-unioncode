#include "handler.h"

Handler::Handler()
    : QObject()
{

}

Handler::~Handler()
{
    if (device) {
        auto process = qobject_cast<QProcess *> (device);
        if (process) {
            process->kill();
            process->close();
            delete process;
            device = nullptr;
        }
    }
    if (device) {
        auto socket = qobject_cast<QTcpSocket *>(device);
        if (socket) {
            socket->disconnectFromHost();
            socket->close();
            delete socket;
            device = nullptr;
        }
    }
}

void Handler::bind(QProcess *qIODevice)
{
    device = qIODevice;
    QObject::connect(qIODevice, &QProcess::readyRead,
                     this, &Handler::doReadAll,
                     Qt::UniqueConnection);
}

void Handler::bind(QTcpSocket *qIODevice)
{
    device = qIODevice;
    QObject::connect(qIODevice, &   QTcpSocket::readyRead,
                     this, &Handler::doReadAll,
                     Qt::UniqueConnection);
}

void Handler::nowToWrite(const QByteArray &array)
{
    if (device) {
        device->write(array);
        device->waitForBytesWritten(3000);
    }
}

void Handler::doReadAll()
{
    auto array = device->readAll();
    filterData(array);
    emit nowReadedAll(array);
}
