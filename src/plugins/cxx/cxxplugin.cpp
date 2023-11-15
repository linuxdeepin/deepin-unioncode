// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cxxplugin.h"

#include "cmake/cmakegenerator.h"
#include "ninja/ninjagenerator.h"
#include "cmake/builder/mainframe/cmakebuildergenerator.h"
#include "cmake/option/optioncmakegenerator.h"
#include "cmake/project/mainframe/properties/targetsmanager.h"
#include "cmake/project/mainframe/cmakeprojectgenerator.h"
#include "ninja/project/ninjaprojectgenerator.h"
#include "ninja/builder/mainframe/ninjabuildergenerator.h"
#include "ninja/option/optionninjagenerator.h"

#include "services/language/languageservice.h"
#include "services/project/projectservice.h"
#include "services/option/optionservice.h"

using namespace dpfservice;

void CxxPlugin::initialize()
{
    qInfo() << __FUNCTION__;
}

bool CxxPlugin::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    LanguageService *languageService = ctx.service<LanguageService>(LanguageService::name());
    if (languageService) {
        QString errorString;
        bool ret = languageService->regClass<CMakeGenerator>(CMakeGenerator::toolKitName(), &errorString);
        if (!ret) {
            qCritical() << errorString;
        } else {
            ret = languageService->create<CMakeGenerator>(CMakeGenerator::toolKitName(), &errorString);
            if (!ret) {
                qCritical() << errorString;
            }
        }

        ret = languageService->regClass<NinjaProjectGenerator>(NinjaProjectGenerator::toolKitName(), &errorString);
        if (!ret) {
            qCritical() << errorString;
        } else {
            ret = languageService->create<NinjaProjectGenerator>(NinjaProjectGenerator::toolKitName(), &errorString);
            if (!ret) {
                qCritical() << errorString;
            }
        }
    }

    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        QString errorString;
        projectService->implGenerator<CmakeProjectGenerator>(CmakeProjectGenerator::toolKitName(), &errorString);
    }

    if (projectService && !projectService->getActiveTarget) {
        using namespace std::placeholders;
        projectService->getActiveTarget = std::bind(&TargetsManager::getActivedTargetByTargetType, TargetsManager::instance(), _1);
    }
    if (projectService) {
        QString errorString;
        projectService->implGenerator<NinjaProjectGenerator>(NinjaProjectGenerator::toolKitName(), &errorString);
    }

    OptionService *optionService = ctx.service<OptionService>(OptionService::name());
    if (!optionService) {
        qCritical() << "Failed, not found option service!";
        abort();
    }
    optionService->implGenerator<OptionCmakeGenerator>(OptionCmakeGenerator::kitName());
    optionService->implGenerator<OptionNinjaGenerator>(OptionNinjaGenerator::kitName());

    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (builderService) {
        QString errorString;
        bool retCmake = builderService->regClass<CMakeBuilderGenerator>(CMakeBuilderGenerator::toolKitName(), &errorString);
        if (retCmake) {
            builderService->create<CMakeGenerator>(CMakeGenerator::toolKitName(), &errorString);
        }
        bool retNinja = builderService->regClass<NinjaBuilderGenerator>(NinjaBuilderGenerator::toolKitName(), &errorString);
        if (retNinja) {
            builderService->create<NinjaBuilderGenerator>(NinjaBuilderGenerator::toolKitName(), &errorString);
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag CxxPlugin::stop()
{
    return Sync;
}


