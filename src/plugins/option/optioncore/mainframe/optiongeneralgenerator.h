// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONGENERALGENERATOR_H
#define OPTIONGENERALGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionGeneralGenerator : public dpfservice::OptionGenerator
{
    Q_OBJECT
public:
    OptionGeneralGenerator();
    inline static QString kitName() {return QObject::tr("General");}
    virtual QWidget *optionWidget() override;
};

#endif // OPTIONGENERALGENERATOR_H
