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
#include "pythonplugin.h"
#include "python/pythongenerator.h"
#include "services/language/languageservice.h"

using namespace dpfservice;

void PythonPlugin::initialize()
{
    qInfo() << __FUNCTION__;
}

bool PythonPlugin::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    LanguageService *languageService = ctx.service<LanguageService>(LanguageService::name());
    if (languageService) {
        QString errorString;
        bool ret = languageService->regClass<PythonGenerator>(PythonGenerator::toolKitName(), &errorString);
        if (!ret) {
            qCritical() << errorString;
        } else {
            ret = languageService->create<PythonGenerator>(PythonGenerator::toolKitName(), &errorString);
            if (!ret) {
                qCritical() << errorString;
            }
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag PythonPlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}


