#ifndef DEBUGGERSERVICE_H
#define DEBUGGERSERVICE_H

#include <framework/framework.h>

namespace dpfservice {

class DebuggerService final : public dpf::PluginService, dpf::AutoServiceRegister<DebuggerService>
{
public:
    explicit DebuggerService(QObject *parent = nullptr);
    static QString name()
    {
        return "org.deepin.service.DebuggerService";
    }
};

} // namespace dpfservice
#endif // DEBUGGERSERVICE_H
