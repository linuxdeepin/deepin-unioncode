// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONGRADLEGENERATOR_H
#define OPTIONGRADLEGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionGradleGeneratorPrivate;
class OptionGradleGenerator : public dpfservice::OptionGenerator
{
public:
    explicit OptionGradleGenerator();
    ~OptionGradleGenerator() override;

    inline static QString kitName() {return "Gradle";}
    virtual QWidget *optionWidget() override;

private:
    OptionGradleGeneratorPrivate *const d;
};

#endif // OPTIONGRADLEGENERATOR_H
