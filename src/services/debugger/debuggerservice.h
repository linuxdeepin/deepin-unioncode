/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DEBUGGERSERVICE_H
#define DEBUGGERSERVICE_H

#include <framework/framework.h>

namespace dpfservice {

class DebuggerService final : public dpf::PluginService,
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
};

} // namespace dpfservice
#endif // DEBUGGERSERVICE_H
