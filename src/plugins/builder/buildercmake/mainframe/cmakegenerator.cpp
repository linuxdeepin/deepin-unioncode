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
#include "cmakegenerator.h"
#include "cmakemanager.h"
#include "services/window/windowservice.h"
#include "services/builder/builderservice.h"

#include <QPushButton>

using namespace dpfservice;

class CMakeGeneratorPrivate
{
    friend class CMakeGenerator;

};

CMakeGenerator::CMakeGenerator()
    : d(new CMakeGeneratorPrivate())
{
    auto &ctx = dpfInstance.serviceContext();
    auto builderService = ctx.service<BuilderService>(BuilderService::name());
    if (!builderService) {
        qCritical() << "Failed, not found service : builderService";
        abort();
    }

    QObject::connect(CMakeManager::instance(), &CMakeManager::addCompileOutput,
                     [=](const QString &content, OutputFormat outputFormat) {
        builderService->interface.compileOutput(content, outputFormat);
    });

    QObject::connect(CMakeManager::instance(), &CMakeManager::addProblemOutput,
                     [=](const Task &task, int linkedOutputLines, int skipLines) {
        builderService->interface.problemOutput(task, linkedOutputLines, skipLines);
    });

    QObject::connect(CMakeManager::instance(), &CMakeManager::buildStateChanged,
                     [=](BuildState state, QString originCmds) {
        builderService->interface.buildStateChanged(state, originCmds);
    });
}

CMakeGenerator::~CMakeGenerator()
{
    if (d)
        delete d;
}

