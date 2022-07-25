/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "codeportingmanager.h"
#include "configwidget.h"
#include "common/widget/outputpane.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"

#include <QtConcurrent>

#define AsynInvoke(Fun)       \
    QtConcurrent::run([=]() { \
        Fun;                  \
    });

using namespace dpfservice;
CodePortingManager *CodePortingManager::instance()
{
    static CodePortingManager ins;
    return &ins;
}

OutputPane *CodePortingManager::getOutputPane() const
{
    return outputPane;
}

void CodePortingManager::slotShowConfigWidget()
{
    if (cfgWidget) {
        cfgWidget->setModal(true);
        cfgWidget->show();
    }
}

void CodePortingManager::slotPortingStart(const QString &project, const QString &srcCPU, const QString &destCPU)
{
    if (codeporting.isRunning())
        return;

    resetUI();

    QString projectSrcPath;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && projectService->projectView.getAllProjectInfo) {
        auto allInfo = projectService->projectView.getAllProjectInfo();
        if (allInfo.isEmpty()) {
            return;
        } else {
            for (auto projInfo : allInfo) {
                QString path = projInfo.workspaceFolder();
                QString dirName = path.split("/").back();
                if (dirName == project) {
                    projectSrcPath = path;
                    break;
                }
            }
        }
    }
    AsynInvoke(codeporting.start(projectSrcPath, srcCPU, destCPU));
}

void CodePortingManager::slotAppendOutput(const QString &text, OutputPane::OutputFormat format)
{
    if (outputPane) {
        QString newContent = text;
        if (OutputPane::OutputFormat::NormalMessage == format
                || OutputPane::OutputFormat::ErrorMessage == format) {
            QDateTime curDatetime = QDateTime::currentDateTime();
            QString time = curDatetime.toString("hh:mm:ss");
            newContent = time + ": " + newContent;
        }
        outputPane->appendText(newContent, format);
    }
}

CodePortingManager::CodePortingManager(QObject *parent)
    : QObject(parent), cfgWidget(new ConfigWidget), outputPane(new OutputPane)
{
    qRegisterMetaType<OutputPane::OutputFormat>("OutputPane::OutputFormat");

    connect(cfgWidget, &ConfigWidget::sigStartPorting, this, &CodePortingManager::slotPortingStart);
    connect(&codeporting, &CodePorting::outputInformation, this, &CodePortingManager::slotAppendOutput);
}

CodePortingManager::~CodePortingManager()
{
    if (cfgWidget) {
        delete cfgWidget;
        cfgWidget = nullptr;
    }

    if (outputPane) {
        delete outputPane;
        outputPane = nullptr;
    }
}

void CodePortingManager::resetUI()
{
    outputPane->clearContents();

    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService) {
        windowService->switchWidgetContext("Code Porting");
    }
}
