#ifndef FILEBROWSERRECEIVER_H
#define FILEBROWSERRECEIVER_H

#include <framework/framework.h>

class FileBrowserReceiver: public dpf::EventHandler, dpf::AutoEventHandlerRegister<FileBrowserReceiver>
{
    friend class dpf::AutoEventHandlerRegister<FileBrowserReceiver>;
public:
    explicit FileBrowserReceiver(QObject * parent = nullptr);

    static Type type()
    {
        return dpf::EventHandler::Type::Async;
    }

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

#endif // FILEBROWSERRECEIVER_H
