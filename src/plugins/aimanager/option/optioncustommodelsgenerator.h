// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONCODEGEEXGENERATOR_H
#define OPTIONCODEGEEXGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionCustomModelsGenerator : public dpfservice::OptionGenerator
{
public:
    OptionCustomModelsGenerator();
    inline static QString kitName() { return "Models"; }
    virtual QWidget *optionWidget() override;
};

#endif // OPTIONCODEGEEXGENERATOR_H
