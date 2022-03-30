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
        qCritical() << event;
        abort();
    }
    if (event.topic() == T_MENU) {
        if (event.data() == D_FILE_OPENDOCUMENT) {
            return RecentProxy::instance()->addDocument(event.property(P_FILEPATH).toString());
        } else if (event.data() == D_FILE_OPENFOLDER) {
            return RecentProxy::instance()->addFolder(event.property(P_FILEPATH).toString());
        }
    }
}

RecentProxy *RecentProxy::instance()
{
    static RecentProxy ins;
    return &ins;
}
