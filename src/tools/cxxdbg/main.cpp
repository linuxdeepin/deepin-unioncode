#include "server.h"
#include <QCoreApplication>
#include <QThread>
#include <QDebug>

// default, listen on port 4711,
// which same as VScode doing.
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    cxxdbg::Server server;

    qInfo() << "Listening on Port: 4711\n";
    QObject::connect(&a, &QCoreApplication::aboutToQuit, [&server](){
        server.stop();
    });

    server.start();
    return a.exec();
}
