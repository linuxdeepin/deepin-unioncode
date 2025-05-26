// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pythonprojectgenerator.h"
#include "python/project/properties/configpropertywidget.h"
#include "python/project/properties/configutil.h"
#include "common/dialog/propertiesdialog.h"
#include "services/window/windowservice.h"

using namespace config;
class PythonProjectGeneratorPrivate
{
    friend class PythonProjectGenerator;
    QMenu *pythonMenu { nullptr };
};

PythonProjectGenerator::PythonProjectGenerator()
    : d(new PythonProjectGeneratorPrivate())
{
    using namespace dpfservice;
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (!projectService) {
        qCritical() << "Failed, not found service : projectService";
        abort();
    }
}

PythonProjectGenerator::~PythonProjectGenerator()
{
    qInfo() << __FUNCTION__;

    if (d)
        delete d;
}

QStringList PythonProjectGenerator::supportLanguages()
{
    return { dpfservice::MWMFA_PYTHON };
}

DWidget *PythonProjectGenerator::configureWidget(const QString &language,
                                                 const QString &projectPath)
{
    using namespace dpfservice;

    prjInfo.setLanguage(language);
    prjInfo.setKitName(PythonProjectGenerator::toolKitName());
    prjInfo.setWorkspaceFolder(projectPath);
    prjInfo.setExePrograms({ exeCurrent, exeEntry });

    return nullptr;
}

bool PythonProjectGenerator::configure(const dpfservice::ProjectInfo &projectInfo)
{
    DirectoryGenerator::configure(projectInfo);
    ProjectConfigure *param = ConfigUtil::instance()->getConfigureParamPointer();
    ConfigUtil::instance()->readConfig(ConfigUtil::instance()->getConfigPath(projectInfo.workspaceFolder()), *param);

    return true;
}

QMenu *PythonProjectGenerator::createItemMenu(const QStandardItem *item)
{
    if (item->parent())
        return nullptr;

    QMenu *menu = new QMenu();
    dpfservice::ProjectInfo info = dpfservice::ProjectInfo::get(item);
    if (info.isEmpty())
        return nullptr;

    QStandardItem *itemTemp = const_cast<QStandardItem *>(item);
    if (!itemTemp)
        return nullptr;

    QAction *action = new QAction(tr("Properties"));
    menu->addAction(action);
    QObject::connect(action, &QAction::triggered, this, [=]() {
        actionProperties(info, itemTemp);
    });

    return menu;
}

void PythonProjectGenerator::doPythonCleanMenu()
{
    if (d->pythonMenu) {
        for (auto &action : d->pythonMenu->actions()) {
            d->pythonMenu->removeAction(action);
        }
    }
}

void PythonProjectGenerator::actionProperties(const dpfservice::ProjectInfo &info, QStandardItem *item)
{
    PropertiesDialog dlg;
    ConfigPropertyWidget *property = new ConfigPropertyWidget(info, item);
    dlg.insertPropertyPanel(tr("Config"), property);
    dlg.exec();
}
