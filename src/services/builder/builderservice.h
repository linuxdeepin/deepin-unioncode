/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#ifndef BUILDERSERVICE_H
#define BUILDERSERVICE_H

#include <framework/framework.h>

namespace dpfservice {

class BuilderService final : public dpf::PluginService,
        dpf::AutoServiceRegister<BuilderService>
{
    Q_OBJECT
    Q_DISABLE_COPY(BuilderService)
public:
    explicit BuilderService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    static QString name()
    {
        return "org.deepin.service.BuilderService";
    }

signals:
    /*!
     * \brief builderCommand 工程目标执行指令, 调用构建系统的入口
     * \param program 程序
     * \param arguments 程序参数
     */
    void builderCommand(const QString &program, const QStringList &arguments);
};

} // namespace dpfservice
#endif // BUILDERSERVICE_H
