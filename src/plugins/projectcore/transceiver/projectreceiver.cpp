#include "projectreceiver.h"
#include "common/common.h"

static QStringList subTopics
{
    T_MENU
};

ProjectReceiver::ProjectReceiver(QObject *parent)
    : dpf::EventHandler (parent)
    , dpf::AutoEventHandlerRegister<ProjectReceiver> ()
{

}

dpf::EventHandler::Type ProjectReceiver::type()
{
    return dpf::EventHandler::Type::Sync;
}

QStringList ProjectReceiver::topics()
{
    return subTopics; //绑定menu 事件
}

void ProjectReceiver::eventProcess(const dpf::Event &event)
{
    if (!subTopics.contains(event.topic())) {
        qCritical() << event;
        abort();
    }
}


