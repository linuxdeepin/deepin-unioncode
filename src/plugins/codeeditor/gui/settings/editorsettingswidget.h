// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORSETTINGSWIDGET_H
#define EDITORSETTINGSWIDGET_H

#include "common/widget/pagewidget.h"
#include "services/option/optiongenerator.h"

class EditorSettingsWidgetGenerator : public dpfservice::OptionGenerator
{
    Q_OBJECT
public:
    explicit EditorSettingsWidgetGenerator() = default;

    inline static QString kitName() { return tr("Editor"); }
    virtual QWidget *optionWidget() override;
};

class EditorSettingsWidgetPrivate;
class EditorSettingsWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit EditorSettingsWidget(QWidget *parent = nullptr);
    ~EditorSettingsWidget() override;

    virtual void saveConfig() override;
    virtual void readConfig() override;

private:
    EditorSettingsWidgetPrivate *const d;
};

#endif   // EDITORSETTINGSWIDGET_H
