// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "documentfindmodule.h"
#include "find/findtoolbar.h"

DocumentFindModule::DocumentFindModule()
{
}

DocumentFindModule::~DocumentFindModule()
{
    if (findToolBar)
        delete findToolBar;
}

void DocumentFindModule::initialize(Controller *uiController)
{
    Q_UNUSED(uiController)

    findToolBar = new FindToolBar;
}
