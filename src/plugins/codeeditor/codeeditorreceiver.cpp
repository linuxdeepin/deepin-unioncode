#include "codeeditorreceiver.h"

static QStringList subTopics{"Menu", "FileBrowser"};

CodeEditorReceiver::CodeEditorReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<CodeEditorReceiver> ()
{

}

dpf::EventHandler::Type CodeEditorReceiver::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList CodeEditorReceiver::topics()
{
    return subTopics; //绑定menu 事件
}

void CodeEditorReceiver::eventProcess(const dpf::Event &event)
{
    if (!subTopics.contains(event.topic()))
        abort();

    if (event.topic() == "FileBrowser") {
        if(event.data() == "TreeView.Item.doubleClicked") {
            return DpfEventMiddleware::instance().toOpenFile
                    (event.property("FilePath").toString());
        }
    }

    if (event.topic() == "Menu") {
        if (event.data() == "File.OpenDocument") {
            return DpfEventMiddleware::instance().toOpenFile
                    (event.property("FilePath").toString());
        }
        if (event.data() == "File.OpenFolder")
            qInfo() << event;
    }
}

DpfEventMiddleware &DpfEventMiddleware::instance()
{
    static DpfEventMiddleware ins;
    return ins;
}
