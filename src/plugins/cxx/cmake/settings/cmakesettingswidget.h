// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKESETTINGSWIDGET_H
#define CMAKESETTINGSWIDGET_H

#include "global_define.h"

#include "common/widget/pagewidget.h"
#include "services/option/optiongenerator.h"

class CMakeSettingsWidgetGenerator : public dpfservice::OptionGenerator
{
    Q_OBJECT
public:
    explicit CMakeSettingsWidgetGenerator() = default;

    inline static QString kitName() { return CMakeKitName; }
    virtual QWidget *optionWidget() override;
};

class CMakeSettingsWidgetPrivate;
class CMakeSettingsWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit CMakeSettingsWidget(QWidget *parent = nullptr);
    ~CMakeSettingsWidget() override;

    virtual void saveConfig() override;
    virtual void readConfig() override;

private:
    CMakeSettingsWidgetPrivate *const d;
};

#endif   // CMAKESETTINGSWIDGET_H
