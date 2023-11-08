// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONCODEGEEXGENERATOR_H
#define OPTIONCODEGEEXGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionCodeGeeXGenerator : public dpfservice::OptionGenerator
{
public:
    OptionCodeGeeXGenerator();
    inline static QString kitName() {return "CodeGeeX";}
    virtual QWidget *optionWidget() override;
};

#endif // OPTIONCODEGEEXGENERATOR_H
