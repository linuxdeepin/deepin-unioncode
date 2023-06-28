// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONPYTHONGENERATOR_H
#define OPTIONPYTHONGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionPythonGeneratorPrivate;
class OptionPythonGenerator : public dpfservice::OptionGenerator
{
public:
    OptionPythonGenerator();
    inline static QString kitName() {return "Python";}
    virtual QWidget *optionWidget() override;

private:
    OptionPythonGeneratorPrivate *const d;
};

#endif // OPTIONPYTHONGENERATOR_H
