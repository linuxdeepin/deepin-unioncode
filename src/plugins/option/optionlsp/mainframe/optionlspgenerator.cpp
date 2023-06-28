// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionlspgenerator.h"
#include "managerlsptabwidget.h"

OptionLspGenerator::OptionLspGenerator()
{

}

QWidget *OptionLspGenerator::optionWidget()
{
    return new ManagerLspTabWidget;
}
