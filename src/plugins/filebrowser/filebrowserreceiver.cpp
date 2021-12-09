#include "filebrowserreceiver.h"
#include "treeproxy.h"

static QStringList subTopics{"Menu"};

FileBrowserReceiver::FileBrowserReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<FileBrowserReceiver> ()
{

}

QStringList FileBrowserReceiver::topics()
{
    return subTopics; //绑定menu 事件
}

void FileBrowserReceiver::eventProcess(const dpf::Event &event)
{
    if (!subTopics.contains(event.topic()))
        abort();
    qInfo() << event;
    if (event.topic() == "Menu") {
        if (event.data() == "File.OpenDocument")
            return TreeProxy::instance().appendFile
                    (event.property("FilePath").toString());
        if (event.data() == "File.OpenFolder")
            return TreeProxy::instance().appendFolder
                    (event.property("FilePath").toString());
    }

}
