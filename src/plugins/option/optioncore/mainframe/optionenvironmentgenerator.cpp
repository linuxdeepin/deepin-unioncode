// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionenvironmentgenerator.h"
#include "environmentwidget.h"

OptionEnvironmentGenerator::OptionEnvironmentGenerator()
{
}

QWidget *OptionEnvironmentGenerator::optionWidget()
{
    return new EnvironmentWidget();
}
