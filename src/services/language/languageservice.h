// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LANGUAGESERVICE_H
#define LANGUAGESERVICE_H

#include "languagegenerator.h"
#include <framework/framework.h>

namespace dpfservice {

class LanguageService final : public dpf::PluginService,
        dpf::AutoServiceRegister<LanguageService>,
        dpf::QtClassFactory<LanguageGenerator>,
        dpf::QtClassManager<LanguageGenerator>
{
    Q_OBJECT
    Q_DISABLE_COPY(LanguageService)
public:
    explicit LanguageService(QObject *parent = nullptr)
        : dpf::PluginService (parent)
    {

    }

    static QString name()
    {
        return "org.deepin.service.LanguageService";
    }

    template<class T>
    bool regClass(const QString &name, QString *error= nullptr) {
        return dpf::QtClassFactory<LanguageGenerator>::regClass<T>(name, error);
    }

    template<class T>
    T* create(const QString &name, QString *error = nullptr) {
        auto value = dpf::QtClassManager<LanguageGenerator>::value(name);
        if (!value) {
            value = dpf::QtClassFactory<LanguageGenerator>::create(name);
            dpf::QtClassManager<LanguageGenerator>::append(name, value, error);
        }
        return dynamic_cast<T*>(value);
    }
};

} // namespace dpfservice
#endif // LANGUAGESERVICE_H
