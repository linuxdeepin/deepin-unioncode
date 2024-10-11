// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUTSETTINGWIDGET_H
#define SHORTCUTSETTINGWIDGET_H

#include "common/widget/pagewidget.h"
#include "services/option/optiongenerator.h"

class OptionShortcutsettingGenerator : public dpfservice::OptionGenerator
{
    Q_OBJECT
public:
    inline static QString kitName() { return QObject::tr("Commands"); }
    virtual QWidget *optionWidget() override;
};

class ShortcutSettingWidgetPrivate;
class ShortcutSettingWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit ShortcutSettingWidget(QWidget *parent = nullptr);
    virtual ~ShortcutSettingWidget();

    void readConfig() override;
    void saveConfig() override;

protected:
    void hideEvent(QHideEvent *e) override;

private:
    ShortcutSettingWidgetPrivate *const d;
};

#endif   // SHORTCUTSETTINGWIDGET_H
