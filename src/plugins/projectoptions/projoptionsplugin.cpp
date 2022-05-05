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
#include "projoptionsplugin.h"

#include "services/project/projectservice.h"
#include "optionsdialog.h"

#include <QAction>

using namespace dpfservice;

void ProjOptionsPlugin::initialize()
{

}

bool ProjOptionsPlugin::start()
{
    qInfo() << __FUNCTION__;

    optionsDlg = new OptionsDialog();

    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    if (projectService) {
        using namespace std::placeholders;
        if (!projectService->insertOptionPanel) {
            projectService->insertOptionPanel = std::bind(&OptionsDialog::insertOptionPanel, optionsDlg, _1, _2);
        }

        if (!projectService->showProjectOptionsDlg) {
            projectService->showProjectOptionsDlg = std::bind(&OptionsDialog::showProjectOptionsDlg, optionsDlg, _1, _2);
        }
    }
    return true;
}

dpf::Plugin::ShutdownFlag ProjOptionsPlugin::stop()
{
    qInfo() << __FUNCTION__;

    if (optionsDlg) {
        delete optionsDlg;
        optionsDlg = nullptr;
    }
    return Sync;
}
