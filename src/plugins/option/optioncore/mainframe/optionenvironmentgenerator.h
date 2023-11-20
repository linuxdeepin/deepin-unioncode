// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONENVIRONMENTGENERATOR_H
#define OPTIONENVIRONMENTGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionEnvironmentGenerator : public dpfservice::OptionGenerator
{
    Q_OBJECT
public:
    OptionEnvironmentGenerator();
    inline static QString kitName() {return QObject::tr("Environment");}
    virtual QWidget *optionWidget() override;
};

#endif // OPTIONENVIRONMENTGENERATOR_H
