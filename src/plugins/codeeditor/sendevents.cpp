#include "sendevents.h"
#include "framework.h"
#include "common/util/eventdefinitions.h"

void SendEvents::marginDebugPointAdd(const QString filePath, quint64 fileLine)
{
    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_MARGIN_DEBUG_POINT_ADD);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_FILELINE, fileLine);
    dpf::EventCallProxy::instance().pubEvent(event);
    qInfo() << event;
}

void SendEvents::marginDebugPointRemove(const QString filePath, quint64 fileLine)
{
    dpf::Event event;
    event.setTopic(T_CODEEDITOR);
    event.setData(D_MARGIN_DEBUG_POINT_REMOVE);
    event.setProperty(P_FILEPATH, filePath);
    event.setProperty(P_FILELINE, fileLine);
    dpf::EventCallProxy::instance().pubEvent(event);
    qInfo() << event;
}
