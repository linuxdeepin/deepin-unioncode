// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "binarytools.h"
#include "configure/binarytoolsmanager.h"

void BinaryTools::initialize()
{
}

bool BinaryTools::start()
{
    BinaryToolsManager::instance()->setupToolMenu();
    const auto &tools = BinaryToolsManager::instance()->tools();
    BinaryToolsManager::instance()->checkAndAddToToolbar(tools);
    BinaryToolsManager::instance()->updateToolMenu(tools);

    return true;
}

dpf::Plugin::ShutdownFlag BinaryTools::stop()
{
    return Sync;
}
