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
#ifndef TOOLCHECKSERVICE_H
#define TOOLCHECKSERVICE_H

#include <framework/framework.h>

#include <QMap>

namespace dpfservice {
// service interface
class ToolCheckerSevice final : public dpf::PluginService,
        dpf::AutoServiceRegister<ToolCheckerSevice>
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolCheckerSevice)
public:
    static QString name()
    {
        return "org.deepin.service.ToolCheckSevice";
    }

    explicit ToolCheckerSevice(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    /*!
     * \brief checkLanguageBackend 插件语法后端
     * \param language [C/C++][Java][Python]
     */
    DPF_INTERFACE(void, checkLanguageBackend, const QString &language);
};

} // namespace dpfservice

#endif // TOOLCHECKSERVICE_H
