// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGGERSERVICE_H
#define DEBUGGERSERVICE_H

#include <framework/framework.h>
#include "base/abstractdebugger.h"
#include "services/services_global.h"

namespace dpfservice {

class SERVICE_EXPORT DebuggerService final : public dpf::PluginService,
        dpf::AutoServiceRegister<DebuggerService>
{
    Q_OBJECT
    Q_DISABLE_COPY(DebuggerService)

public:
    explicit DebuggerService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    static QString name()
    {
        return "org.deepin.service.DebuggerService";
    }

    /**
     * @brief runCoredump launch debugger and debug coredump file.
     * @param target debugee file path.
     * @param core coredump file path.
     * @param kit current project kit.
     */
    DPF_INTERFACE(bool, runCoredump, const QString &target, const QString &core, const QString &kit);

    /**
     * @brief DPF_INTERFACE
     * @param kit
     * @param debugger
     */
    DPF_INTERFACE(void, registerDebugger, const QString &kit, AbstractDebugger *debugger);

    DPF_INTERFACE(AbstractDebugger::RunState, getDebugState);
};

} // namespace dpfservice
#endif // DEBUGGERSERVICE_H
