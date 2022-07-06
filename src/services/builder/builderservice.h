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

#include "buildergenerator.h"
#include "builderglobals.h"
#include "builderinterface.h"
#include <framework/framework.h>

namespace dpfservice {

class BuilderService final : public dpf::PluginService,
        dpf::AutoServiceRegister<BuilderService>,
        dpf::QtClassFactory<BuilderGenerator>,
        dpf::QtClassManager<BuilderGenerator>
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

    template<class T>
    bool regClass(const QString &name, QString *error= nullptr) {
        return dpf::QtClassFactory<BuilderGenerator>::regClass<T>(name, error);
    }

    template<class T>
    T* create(const QString &name, QString *error = nullptr) {
        auto value = dpf::QtClassManager<BuilderGenerator>::value(name);
        if (!value) {
            value = dpf::QtClassFactory<BuilderGenerator>::create(name);
            dpf::QtClassManager<BuilderGenerator>::append(name, value, error);
        }
        return dynamic_cast<T*>(value);
    }

    template<class T>
    QString name(T* value) const{
        return QtClassManager<T>::key(value);
    }

    template<class T>
    QList<T*> values() const {
        return QtClassManager<T>::values();
    }

    BuilderInterface interface;
};

} // namespace dpfservice
#endif // BUILDERSERVICE_H
