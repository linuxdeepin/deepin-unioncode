// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONJAVAGENERATOR_H
#define OPTIONJAVAGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionJavaGenerator : public dpfservice::OptionGenerator
{
public:
    OptionJavaGenerator();
    inline static QString kitName() {return "Java";}
    virtual QWidget *optionWidget() override;
};

#endif // OPTIONJAVAGENERATOR_H
