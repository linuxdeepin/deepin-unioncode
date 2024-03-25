// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONCMAKEGENERATOR_H
#define OPTIONCMAKEGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionCmakeGeneratorPrivate;
class OptionCmakeGenerator : public dpfservice::OptionGenerator
{
public:
    explicit OptionCmakeGenerator();
    ~OptionCmakeGenerator() override;

    inline static QString kitName() {return "CMake";}
    virtual QWidget *optionWidget() override;

private:
    OptionCmakeGeneratorPrivate *const d;
};

#endif // OPTIONCMAKEGENERATOR_H
