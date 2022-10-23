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
