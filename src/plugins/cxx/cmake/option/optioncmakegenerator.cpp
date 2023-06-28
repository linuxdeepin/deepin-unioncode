// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optioncmakegenerator.h"
#include "cmakeoptionwidget.h"

class OptionCmakeGeneratorPrivate
{
    friend class OptionCmakeGenerator;

    CMakeOptionWidget *widget = nullptr;
};

OptionCmakeGenerator::OptionCmakeGenerator()
    : d(new OptionCmakeGeneratorPrivate())
{
    d->widget = new CMakeOptionWidget();
}

QWidget *OptionCmakeGenerator::optionWidget()
{
    return d->widget;
}
