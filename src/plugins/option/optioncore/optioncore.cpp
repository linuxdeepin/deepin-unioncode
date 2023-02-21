/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#include "optioncore.h"
#include "mainframe/optiondefaultkeeper.h"
#include "mainframe/optiongeneralgenerator.h"

#include "common/common.h"
#include "base/abstractwidget.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/option/optionservice.h"

#include "framework/listener/listener.h"

using namespace dpfservice;
void OptionCore::initialize()
{
    auto &ctx = dpfInstance.serviceContext();
    QString errStr;
    if (!ctx.load(OptionService::name(), &errStr)) {
        qCritical() << errStr;
        abort();
    }
}

bool OptionCore::start()
{
    toolchains::generatGlobalFile();

    auto optionDialog = OptionDefaultKeeper::getOptionDialog();
    if (!optionDialog) {
        qCritical() << "Failed, can't init option dialog!";
        abort();
    }

    optionDialog->setModal(true);

    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    OptionService *optionService = ctx.service<OptionService>(OptionService::name());
    if (optionService) {
        optionService->implGenerator<OptionGeneralGenerator>(OptionGeneralGenerator::kitName());
        auto generator = optionService->createGenerator<OptionGeneralGenerator>(OptionGeneralGenerator::kitName());
        if (generator) {
            auto pageWidget = dynamic_cast<PageWidget*>(generator->optionWidget());
            if (pageWidget) {
                optionDialog->insertOptionPanel(OptionGeneralGenerator::kitName(), pageWidget);
            }
        }
    }

    if (windowService && windowService->addAction && windowService->addToolBarActionItem) {
        auto actionOptions = new QAction(MWMTA_OPTIONS);
        ActionManager::getInstance()->registerAction(actionOptions,
                                                     "Tools.Options",
                                                     MWMTA_OPTIONS,
                                                     QKeySequence(Qt::Modifier::CTRL |
                                                                  Qt::Modifier::SHIFT |
                                                                  Qt::Key::Key_H),
                                                     ":/optioncore/images/setting.png");
        windowService->addAction(MWM_TOOLS, new AbstractAction(actionOptions));
        windowService->addToolBarActionItem("Options", actionOptions, "Options");
        windowService->addToolBarSeparator("Options");

        QObject::connect(actionOptions, &QAction::triggered,
                         optionDialog, &QDialog::show);
    }

    DPF_USE_NAMESPACE
    QObject::connect(&Listener::instance(), &Listener::pluginsStarted, [=](){
        if (optionDialog) {
            auto list = optionService->supportGeneratorName<OptionGenerator>();
            for (auto name : list) {
                if (0 == name.compare(OptionGeneralGenerator::kitName()))
                    continue;
                auto generator = optionService->createGenerator<OptionGenerator>(name);
                if (generator) {
                    PageWidget *optionWidget = dynamic_cast<PageWidget*>(generator->optionWidget());
                    if (optionWidget) {
                        optionDialog->insertOptionPanel(name, optionWidget);
                        optionWidget->readConfig();
                        optionWidget->saveConfig();
                    }
                }
            }
        }
    });

    return true;
}

dpf::Plugin::ShutdownFlag OptionCore::stop()
{
    delete OptionDefaultKeeper::getOptionDialog();
    return Sync;
}
