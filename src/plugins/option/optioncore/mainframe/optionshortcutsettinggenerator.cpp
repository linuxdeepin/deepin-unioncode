// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionshortcutsettinggenerator.h"
#include "shortcutsettingwidget.h"

OptionShortcutsettingGenerator::OptionShortcutsettingGenerator()
{
}

QWidget *OptionShortcutsettingGenerator::optionWidget()
{
    return new ShortcutSettingWidget();
}
