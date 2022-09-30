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
    return { recent.topic };
}

void RecentReceiver::eventProcess(const dpf::Event &event)
{
    if (event.data() == recent.saveOpenedProject.name) {
        QString filePathKey = recent.saveOpenedProject.pKeys[0];
        QString kitNameKey = recent.saveOpenedProject.pKeys[1];
        QString languageKey = recent.saveOpenedProject.pKeys[2];
        QString workspaceFolderKey = recent.saveOpenedProject.pKeys[3];
        return RecentProxy::instance()->saveOpenedProject(
                    event.property(filePathKey).toString(),
                    event.property(kitNameKey).toString(),
                    event.property(languageKey).toString(),
                    event.property(workspaceFolderKey).toString());
    }
}

RecentProxy *RecentProxy::instance()
{
    static RecentProxy ins;
    return &ins;
}
