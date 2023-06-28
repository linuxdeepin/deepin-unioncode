// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optiongeneralgenerator.h"
#include "generaloptionwidget.h"

OptionGeneralGenerator::OptionGeneralGenerator()
{

}

QWidget *OptionGeneralGenerator::optionWidget()
{
    return new GeneralOptionWidget();
}
