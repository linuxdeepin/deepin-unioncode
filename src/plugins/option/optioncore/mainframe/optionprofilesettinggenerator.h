// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONPROFILESETTINGGENERATOR_H
#define OPTIONPROFILESETTINGGENERATOR_H

#include "services/option/optiongenerator.h"

class OptionProfilesettingGenerator : public dpfservice::OptionGenerator
{
    Q_OBJECT
public:
    OptionProfilesettingGenerator();
    inline static QString kitName() {return QObject::tr("Interface");}
    virtual QWidget *optionWidget() override;
};

#endif // OPTIONPROFILESETTINGGENERATOR_H
