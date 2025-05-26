// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUILDERSERVICE_H
#define BUILDERSERVICE_H

#include "buildergenerator.h"
#include "builderglobals.h"
#include "services/services_global.h"

#include <framework/framework.h>

namespace dpfservice {

class SERVICE_EXPORT BuilderService final : public dpf::PluginService,
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

    /*!
     * \brief builderCommand
     * \param commandInfo
     */
    DPF_INTERFACE(bool, runbuilderCommand, const QList<BuildCommandInfo> &commandInfo, bool isSynchronous);
};

} // namespace dpfservice
#endif // BUILDERSERVICE_H
