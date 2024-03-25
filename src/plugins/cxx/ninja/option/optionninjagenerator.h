// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONNINJAGENERATOR_H
#define OPTIONNINJAGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionNinjaGeneratorPrivate;
class OptionNinjaGenerator : public dpfservice::OptionGenerator
{
public:
    explicit OptionNinjaGenerator();
    ~OptionNinjaGenerator() override;

    inline static QString kitName() {return "Ninja";}
    virtual QWidget *optionWidget() override;

private:
    OptionNinjaGeneratorPrivate *const d;
};

#endif // OPTIONNINJAGENERATOR_H
