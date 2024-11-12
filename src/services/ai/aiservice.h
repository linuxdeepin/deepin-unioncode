// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AISERVICE_H
#define AISERVICE_H

#include "services/services_global.h"
#include <framework/framework.h>

#include <QIODevice>

namespace dpfservice {
// service interface
class SERVICE_EXPORT AiService final : public dpf::PluginService, dpf::AutoServiceRegister<AiService>
{
    Q_OBJECT
    Q_DISABLE_COPY(AiService)
public:
    static QString name()
    {
        return "org.deepin.service.AiService";
    }

    explicit AiService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    DPF_INTERFACE(bool, available);
    DPF_INTERFACE(void, askQuestion, const QString &prompt, QIODevice *pipe);  // pipe recevice data from ai
};

} // namespace dpfservice

#endif // AISERVICE_H
