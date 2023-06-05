/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *             hongjinchuan<hongjinchuan@uniontech.com>
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
#include "javaplugin.h"

#include "maven/mavengenerator.h"
#include "maven/option/optionmavengenerator.h"
#include "maven/project/mavenprojectgenerator.h"
#include "maven/builder/mainframe/mavenbuildergenerator.h"
#include "gradle/gradlegenerator.h"
#include "gradle/option/optiongradlegenerator.h"
#include "gradle/project/gradleprojectgenerator.h"
#include "gradle/builder/mainframe/gradlebuildergenerator.h"
#include "java/option/optionjavagenerator.h"

#include "services/language/languageservice.h"
#include "services/project/projectservice.h"
#include "services/option/optionservice.h"
#include "services/builder/builderservice.h"

using namespace dpfservice;

void JavaPlugin::initialize()
{
    qInfo() << __FUNCTION__;
}

bool JavaPlugin::start()
{
    qInfo() << __FUNCTION__;
    auto &ctx = dpfInstance.serviceContext();
    LanguageService *languageService = ctx.service<LanguageService>(LanguageService::name());
    if (languageService) {
        QString errorString;
        bool ret = languageService->regClass<MavenGenerator>(MavenGenerator::toolKitName(), &errorString);
        if (!ret) {
            qCritical() << errorString;
        } else {
            ret = languageService->create<MavenGenerator>(MavenGenerator::toolKitName(), &errorString);
            if (!ret) {
                qCritical() << errorString;
            }
        }

        ret = languageService->regClass<GradleGenerator>(GradleGenerator::toolKitName(), &errorString);
        if (!ret) {
            qCritical() << errorString;
        } else {
            ret = languageService->create<GradleGenerator>(GradleGenerator::toolKitName(), &errorString);
            if (!ret) {
                qCritical() << errorString;
            }
        }
    }

    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        QString errorString;
        projectService->implGenerator<MavenProjectGenerator>(MavenProjectGenerator::toolKitName(), &errorString);
        projectService->implGenerator<GradleProjectGenerator>(GradleProjectGenerator::toolKitName(), &errorString);
    }

    OptionService *optionService = ctx.service<OptionService>(OptionService::name());
    if (!optionService) {
        qCritical() << "Failed, not found OptionGradle service!";
        abort();
    }
    optionService->implGenerator<OptionGradleGenerator>(OptionGradleGenerator::kitName());
    optionService->implGenerator<OptionMavenGenerator>(OptionMavenGenerator::kitName());
    optionService->implGenerator<OptionJavaGenerator>(OptionJavaGenerator::kitName());

    BuilderService *builderService = ctx.service<BuilderService>(BuilderService::name());
    if (builderService) {
        QString errorString;
        bool retMaven = builderService->regClass<MavenBuilderGenerator>(MavenBuilderGenerator::toolKitName(), &errorString);
        if (retMaven) {
            builderService->create<MavenBuilderGenerator>(MavenBuilderGenerator::toolKitName(), &errorString);
        }
        bool retGradle = builderService->regClass<GradleBuilderGenerator>(GradleBuilderGenerator::toolKitName(), &errorString);
        if (retGradle) {
            builderService->create<GradleBuilderGenerator>(GradleBuilderGenerator::toolKitName(), &errorString);
        }
    }

    return true;
}

dpf::Plugin::ShutdownFlag JavaPlugin::stop()
{
    qInfo() << __FUNCTION__;
    return Sync;
}


