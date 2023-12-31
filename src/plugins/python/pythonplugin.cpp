// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pythonplugin.h"
#include "python/pythongenerator.h"
#include "python/project/pythonprojectgenerator.h"
#include "python/option/optionpythongenerator.h"

#include "services/language/languageservice.h"
#include "services/project/projectservice.h"
#include "services/option/optionservice.h"

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

    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        QString errorString;
        projectService->implGenerator<PythonProjectGenerator>(PythonProjectGenerator::toolKitName(), &errorString);
    }

    OptionService *optionService = ctx.service<OptionService>(OptionService::name());
    if (!optionService) {
        qCritical() << "Failed, not found OptionPython service!";
        abort();
    }
    optionService->implGenerator<OptionPythonGenerator>(OptionPythonGenerator::kitName());

    return true;
}

dpf::Plugin::ShutdownFlag PythonPlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}


