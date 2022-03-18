#include "recentreceiver.h"
#include "common/common.h"

static QStringList subTopics
{
    T_MENU
};

RecentReceiver::RecentReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<RecentReceiver> ()
{

}

dpf::EventHandler::Type RecentReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList RecentReceiver::topics()
{
    return subTopics; //绑定menu 事件
}

void RecentReceiver::eventProcess(const dpf::Event &event)
{
    if (!subTopics.contains(event.topic())) {
        qInfo() << event;
        abort();
    }
    if (event.topic() == "Menu") {
        if (event.data() == "File.OpenDocument")
            return RecentProxy::instance()->addDocument(event.property("FilePath").toString());
        if (event.data() == "File.OpenFolder")
            return RecentProxy::instance()->addFolder(event.property("FilePath").toString());
    }
}

RecentProxy *RecentProxy::instance()
{
    static RecentProxy ins;
    return &ins;
}
