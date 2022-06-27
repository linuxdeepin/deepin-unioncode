/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#ifndef RUNTIMESERVICE_H
#define RUNTIMESERVICE_H

#include <framework/framework.h>

namespace dpfservice {
struct Target {
    QString name;
    QString path;
    QString targetID;

    QString buildCommand;
    QStringList buildArguments;
    QString buildTarget;
    bool stopOnError = false;
    bool useDefaultCommand = false;

    QString outputPath;

    // TODO(mozart):tempory
    bool enableEnv;

    bool operator==(const Target &other) const
    {
        if (name == other.name
                && path == other.path)
            return true;

        return false;
    }
};
using Targets = QVector<Target>;

enum TargetType {
    kBuildTarget,
    kRebuildTarget,
    kCleanTarget,
    kActiveExecTarget
};

class RuntimeService final : public dpf::PluginService, dpf::AutoServiceRegister<RuntimeService>
{
    Q_OBJECT
    Q_DISABLE_COPY(RuntimeService)
public:
    static QString name()
    {
        return "org.deepin.service.RuntimeService";
    }

    explicit RuntimeService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    DPF_INTERFACE(Target, getActiveTarget, TargetType);
};

} // namespace dpfservice

#endif // RUNTIMESERVICE_H
