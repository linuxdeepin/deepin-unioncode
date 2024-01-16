// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optioncore.h"
#include "mainframe/optiondefaultkeeper.h"
#include "mainframe/optionenvironmentgenerator.h"
#include "mainframe/optionprofilesettinggenerator.h"
#include "mainframe/optionshortcutsettinggenerator.h"
#include <mainframe/pluginwidget.h>

#include "common/common.h"
#include "base/abstractwidget.h"
#include "base/abstractaction.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/option/optionservice.h"

#include "framework/listener/listener.h"

#include <DToolButton>

static QStringList generalKits {};

using namespace dpfservice;
DWIDGET_USE_NAMESPACE
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
        generalKits << OptionEnvironmentGenerator::kitName() << OptionShortcutsettingGenerator::kitName() << OptionProfilesettingGenerator::kitName();
        optionService->implGenerator<OptionEnvironmentGenerator>(generalKits[0]);
        optionService->implGenerator<OptionShortcutsettingGenerator>(generalKits[1]);
        optionService->implGenerator<OptionProfilesettingGenerator>(generalKits[2]);

        optionDialog->insertLabel(tr("General"));
        for (auto name : generalKits) {
            auto generator = optionService->createGenerator<OptionGenerator>(name);
            if (generator) {
                PageWidget *optionWidget = dynamic_cast<PageWidget *>(generator->optionWidget());
                if (optionWidget)
                    optionDialog->insertOptionPanel(name, optionWidget);
            }
        }
    }

    if (windowService && windowService->addAction) {
        auto actionOptions = new QAction(MWMTA_OPTIONS);
        ActionManager::getInstance()->registerAction(actionOptions,
                                                     "Tools.Options",
                                                     MWMTA_OPTIONS,
                                                     QKeySequence(Qt::Modifier::CTRL |
                                                                  Qt::Modifier::SHIFT |
                                                                  Qt::Key::Key_H),
                                                     QString());
        windowService->addAction(MWM_TOOLS, new AbstractAction(actionOptions));

        QObject::connect(actionOptions, &QAction::triggered,
                         optionDialog, &QDialog::show);

        auto toolBtn = new DToolButton();
        toolBtn->setMinimumSize(QSize(48, 48));
        toolBtn->setIconSize(QSize(20, 20));
        toolBtn->setIcon(QIcon::fromTheme("options_setting"));
        connect(toolBtn, &DToolButton::clicked, optionDialog, &QDialog::show);
        windowService->insertToLeftBarBottom(new AbstractWidget(toolBtn));
    }

    DPF_USE_NAMESPACE
    QObject::connect(&Listener::instance(), &Listener::pluginsStarted, [=](){
        if (optionDialog) {
            auto list = optionService->supportGeneratorName<OptionGenerator>();
            optionDialog->insertLabel(tr("Language"));
            for (auto name : list) {
                if (generalKits.contains(name))
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

            PluginWidget *plugin = new PluginWidget;
            optionDialog->insertLabel(tr("plugin"));
            optionDialog->insertOptionPanel(tr("Installed Plugin") ,plugin);
        }
    });

    return true;
}

dpf::Plugin::ShutdownFlag OptionCore::stop()
{
    return Sync;
}
