// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TERMINALSERVICE_H
#define TERMINALSERVICE_H

#include <framework/framework.h>

namespace dpfservice {
// service interface
class TerminalService final : public dpf::PluginService, dpf::AutoServiceRegister<TerminalService>
{
    Q_OBJECT
    Q_DISABLE_COPY(TerminalService)
public:
    static QString name()
    {
        return "org.deepin.service.TerminalService";
    }

    explicit TerminalService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    /**
     * @brief send command to terminal
     * @param command
     */
    DPF_INTERFACE(void, sendCommand, const QString &command);
    DPF_INTERFACE(void, executeCommand, const QString &name, const QString &program, const QStringList &args, const QString &workingDir);
};

} // namespace dpfservice

#endif // TERMINALSERVICE_H
