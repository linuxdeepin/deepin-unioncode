#include "cmakeopenhandler.h"
#include <QFileDialog>

namespace {
CMakeOpenHandler *ins{ nullptr };
}

CMakeOpenHandler::CMakeOpenHandler(QObject *parent) : QObject(parent)
{

}

CMakeOpenHandler *CMakeOpenHandler::instance()
{
    if (!ins)
        ins = new CMakeOpenHandler;
    return ins;
}

QAction *CMakeOpenHandler::openAction()
{
    auto result = new QAction("cmake");
    QObject::connect(result, &QAction::triggered, [=](){
        QFileDialog fileDialog(nullptr, "Open CMake Project", QDir::homePath(), "CMakeLists.txt");
        if(fileDialog.exec()) {
            CMakeOpenHandler::projectOpened(result->text(), fileDialog.selectedFiles()[0]);
        }
    });
    return result;
}
