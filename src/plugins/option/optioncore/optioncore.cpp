// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optioncore.h"
#include "mainframe/optiondefaultkeeper.h"
#include "mainframe/optionprofilesettinggenerator.h"
#include "mainframe/optionshortcutsettinggenerator.h"

#include "common/common.h"
#include "base/abstractwidget.h"
#include "base/abstractaction.h"
#include "services/window/windowservice.h"
#include "services/project/projectservice.h"
#include "services/option/optionservice.h"
#include "services/option/optiondatastruct.h"

#include "framework/listener/listener.h"

#include <DToolButton>

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
        optionService->implGenerator<OptionShortcutsettingGenerator>(option::GROUP_GENERAL, OptionShortcutsettingGenerator::kitName());
        optionService->implGenerator<OptionProfilesettingGenerator>(option::GROUP_GENERAL, OptionProfilesettingGenerator::kitName());

        using namespace std::placeholders;
        optionService->showOptionDialog = std::bind(&OptionsDialog::showAtItem, OptionDefaultKeeper::getOptionDialog(), _1);
    }

    if (windowService) {
        auto mTool = ActionManager::instance()->actionContainer(M_TOOLS);

        auto actionOptions = new QAction(MWMTA_OPTIONS, this);
        actionOptions->setIcon(QIcon::fromTheme("options_setting"));
        auto cmd = ActionManager::instance()->registerAction(actionOptions, "Tools.Options");
        cmd->setDefaultKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H);
        mTool->addAction(cmd);

        auto actionOptionsImpl = new AbstractAction(actionOptions, this);
        windowService->addNavigationItemToBottom(actionOptionsImpl, Priority::lowest);
        QObject::connect(actionOptions, &QAction::triggered,
                         optionDialog, &QDialog::show);
    }

    DPF_USE_NAMESPACE
    QObject::connect(&Listener::instance(), &Listener::pluginsStarted, [=]() {
        if (optionDialog) {
            auto groups = optionService->generatorGroups();
            if (groups.isEmpty())
                return;

            //raise general and language group
            auto raiseGroupPriority = [=](QStringList &groupList, const QString &groupName) {
                if (groupList.contains(groupName)) {
                    groupList.removeOne(groupName);
                    groupList.insert(0, groupName);
                }
            };

            raiseGroupPriority(groups, option::GROUP_LANGUAGE);
            raiseGroupPriority(groups, option::GROUP_GENERAL);

            for (auto group : groups) {
                optionDialog->insertLabel(group);
                auto generatorNames = optionService->supportGeneratorNameByGroup<OptionGenerator>(group);
                for (auto name : generatorNames) {
                    auto generator = optionService->createGenerator<OptionGenerator>(name);
                    if (generator) {
                        PageWidget *optionWidget = dynamic_cast<PageWidget *>(generator->optionWidget());
                        if (optionWidget) {
                            optionDialog->insertOptionPanel(name, optionWidget);
                            optionWidget->readConfig();
                            optionWidget->saveConfig();
                        }
                    }
                }
            }
        }
    });

    return true;
}

dpf::Plugin::ShutdownFlag OptionCore::stop()
{
    return Sync;
}
