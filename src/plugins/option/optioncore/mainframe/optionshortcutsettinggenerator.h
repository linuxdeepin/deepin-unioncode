// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONSHORTCUTSETTINGGENERATOR_H
#define OPTIONSHORTCUTSETTINGGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionShortcutsettingGenerator : public dpfservice::OptionGenerator
{
    Q_OBJECT
public:
    OptionShortcutsettingGenerator();
    inline static QString kitName() {return QObject::tr("Commands");}
    virtual QWidget *optionWidget() override;
};

#endif // OPTIONSHORTCUTSETTINGGENERATOR_H
