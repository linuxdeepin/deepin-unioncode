#ifndef RECVEVENTS_H
#define RECVEVENTS_H

#include <framework/framework.h>

class RecvEvents: public dpf::EventHandler, dpf::AutoEventHandlerRegister<RecvEvents>
{
    friend class dpf::AutoEventHandlerRegister<RecvEvents>;
public:
    explicit RecvEvents(QObject * parent = nullptr);

    static Type type()
    {
        return dpf::EventHandler::Type::Async;
    }

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

#endif // RECVEVENTS_H
