// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONCMAKEGENERATOR_H
#define OPTIONCMAKEGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionLspGenerator : public dpfservice::OptionGenerator
{
public:
    OptionLspGenerator();
    inline static QString kitName() {return "Lsp";}
    virtual QWidget *optionWidget() override;
};

#endif // OPTIONCMAKEGENERATOR_H
