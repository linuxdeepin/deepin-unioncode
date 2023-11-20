// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionprofilesettinggenerator.h"
#include "profilesettingwidget.h"

OptionProfilesettingGenerator::OptionProfilesettingGenerator()
{
}

QWidget *OptionProfilesettingGenerator::optionWidget()
{
    return new ProfileSettingWidget();
}
