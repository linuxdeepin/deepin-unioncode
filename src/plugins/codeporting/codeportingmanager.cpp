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
#include "reportpane.h"

#include <QtConcurrent>

#define AsynInvoke(Fun)       \
    QtConcurrent::run([=]() { \
        Fun;                  \
    });

static const int kLineNumberAdaptation = 1;
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

QWidget *CodePortingManager::getReportPane() const
{
    return reportPane;
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
    QString buildDir;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService && projectService->getAllProjectInfo) {
        auto allInfo = projectService->getAllProjectInfo();
        if (allInfo.isEmpty()) {
            return;
        } else {
            for (auto projInfo : allInfo) {
                QString path = projInfo.workspaceFolder();
                QString dirName = path.split("/").back();
                if (dirName == project) {
                    projectSrcPath = path;
                    buildDir = projInfo.buildFolder();
                    break;
                }
            }
        }
    }
    AsynInvoke(codeporting.start(projectSrcPath, srcCPU, buildDir, destCPU));
}

void CodePortingManager::slotPortingStatus(CodePorting::PortingStatus status)
{
    if (status == CodePorting::kSuccessful) {
        reportPane->refreshDispaly();
    }
}

void CodePortingManager::slotSelectedChanged(const QString &filePath, const QString &suggestion, int startLine, int endLine)
{
    Q_UNUSED(endLine)

    int startLineInEditor = startLine + kLineNumberAdaptation;
    int endLineInEditor = endLine + kLineNumberAdaptation;
    editor.jumpToLine(filePath, startLineInEditor);
    AnnotationInfo annInfo{AnnotationInfo::Role::get()->Note, suggestion};
    editor.setAnnotation(filePath, startLine, QString("CodePorting"), annInfo);
    QColor backgroundColor(Qt::red);
    backgroundColor.setAlpha(100);
    for (int lineNumber = startLineInEditor; lineNumber <= endLineInEditor; ++lineNumber) {
        editor.setLineBackground(filePath, lineNumber, backgroundColor);
    }
}

void CodePortingManager::slotAppendOutput(const QString &content, OutputPane::OutputFormat format, OutputPane::AppendMode mode)
{
    if (outputPane) {
        QString newContent = content;
        if (OutputPane::OutputFormat::NormalMessage == format
                || OutputPane::OutputFormat::ErrorMessage == format) {
            QDateTime curDatetime = QDateTime::currentDateTime();
            QString time = curDatetime.toString("hh:mm:ss");
            newContent = time + ": " + newContent;
        }
        outputPane->appendText(newContent, format, mode);
    }
}

CodePortingManager::CodePortingManager(QObject *parent)
    : QObject(parent),
      cfgWidget(new ConfigWidget),
      outputPane(new OutputPane),
      reportPane(new ReportPane(&codeporting))
{
    qRegisterMetaType<OutputPane::OutputFormat>("OutputPane::OutputFormat");
    qRegisterMetaType<OutputPane::AppendMode>("OutputPane::AppendMode");
    qRegisterMetaType<CodePorting::PortingStatus>("PortingStatus");

    connect(cfgWidget, &ConfigWidget::sigStartPorting, this, &CodePortingManager::slotPortingStart);
    connect(&codeporting, &CodePorting::outputInformation, this, &CodePortingManager::slotAppendOutput);
    connect(&codeporting, &CodePorting::notifyPortingStatus, this, &CodePortingManager::slotPortingStatus);
    connect(reportPane, &ReportPane::selectedChanged, this, &CodePortingManager::slotSelectedChanged);
}

CodePortingManager::~CodePortingManager()
{
    if (cfgWidget) {
        // delete cfgWidget; mainwindow released
        cfgWidget = nullptr;
    }

    if (outputPane) {
        // delete outputPane; mainwindow released
        outputPane = nullptr;
    }

    if (reportPane) {
        // delete reportPane; mainwindow released
        reportPane = nullptr;
    }
}

void CodePortingManager::resetUI()
{
    outputPane->clearContents();
    editor.switchContext(tr("C&ode Porting"));
}
