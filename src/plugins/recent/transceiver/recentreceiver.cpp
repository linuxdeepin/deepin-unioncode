#include "recentreceiver.h"
#include "common/common.h"

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
    return {
        T_MENU
    };//绑定menu 事件
}

void RecentReceiver::eventProcess(const dpf::Event &event)
{
    if (!topics().contains(event.topic())) {
        qCritical() << event;
        abort();
    }
    if (event.topic() == T_MENU) {
        if (event.data() == D_FILE_OPENDOCUMENT) {
            return RecentProxy::instance()->addDocument(event.property(P_FILEPATH).toString());
        } else if (event.data() == D_FILE_OPENPROJECT) {
            return RecentProxy::instance()->addProject(
                        event.property(P_FILEPATH).toString(),
                        event.property(P_KITNAME).toString(),
                        event.property(P_LANGUAGE).toString(),
                        event.property(P_WORKSPACEFOLDER).toString());
        }
    }
}

RecentProxy *RecentProxy::instance()
{
    static RecentProxy ins;
    return &ins;
}
