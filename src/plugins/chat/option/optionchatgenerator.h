// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONCHATGENERATOR_H
#define OPTIONCHATGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionChatGenerator : public dpfservice::OptionGenerator
{
public:
    OptionChatGenerator();
    inline static QString kitName() {return "Chat";}
    virtual QWidget *optionWidget() override;
};

#endif // optionchatgenerator_H
