#include "recvevents.h"
#include "treeproxy.h"

static QStringList subTopics{"Menu"};

RecvEvents::RecvEvents(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<RecvEvents> ()
{

}

QStringList RecvEvents::topics()
{
    return subTopics; //绑定menu 事件
}

void RecvEvents::eventProcess(const dpf::Event &event)
{
    if (!subTopics.contains(event.topic()))
        abort();
    qInfo() << event;
    if (event.topic() == "Menu") {
        if (event.data() == "File.OpenFile")
            return TreeProxy::instance().appendFile(event.property("Path").toString());
        if (event.data() == "File.OpenDirectory")
            return TreeProxy::instance().appendFolder(event.property("Path").toString());
    }

}
