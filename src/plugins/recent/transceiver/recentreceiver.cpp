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
        QString kitName = event.property(recent.saveOpenedProject.pKeys[0]).toString();
        QString language = event.property(recent.saveOpenedProject.pKeys[1]).toString();
        QString workspace = event.property(recent.saveOpenedProject.pKeys[2]).toString();
        if (QDir(workspace).exists())
            RecentProxy::instance()->saveOpenedProject(kitName, language, workspace);
    } else if (event.data() == recent.saveOpenedFile.name) {
        QString filePath = event.property(recent.saveOpenedFile.pKeys[0]).toString();
        if (QFileInfo(filePath).exists())
            RecentProxy::instance()->saveOpenedFile(filePath);
    }
}

RecentProxy *RecentProxy::instance()
{
    static RecentProxy ins;
    return &ins;
}
