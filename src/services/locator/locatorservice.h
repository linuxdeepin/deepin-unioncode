// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOCATORSERVICE_H
#define LOCATORSERVICE_H

#include "base/abstractlocator.h"

#include <framework/framework.h>

namespace dpfservice {

class LocatorService final : public dpf::PluginService,
        dpf::AutoServiceRegister<LocatorService>
{
    Q_OBJECT
    Q_DISABLE_COPY(LocatorService)

public:
    explicit LocatorService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    static QString name()
    {
        return "org.deepin.service.LocatorService";
    }

    DPF_INTERFACE(void, registerLocator, abstractLocator *locator);
};

} // namespace dpfservice

#endif // LOCATORSERVICE_H
