// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONMAVENGENERATOR_H
#define OPTIONMAVENGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionMavenGeneratorPrivate;
class OptionMavenGenerator : public dpfservice::OptionGenerator
{
public:
    explicit OptionMavenGenerator();
    ~OptionMavenGenerator() override;

    inline static QString kitName() {return "Maven";}
    virtual QWidget *optionWidget() override;

private:
    OptionMavenGeneratorPrivate *const d;
};

#endif // OPTIONMAVENGENERATOR_H
