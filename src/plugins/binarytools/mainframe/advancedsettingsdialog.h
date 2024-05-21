// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADVANCEDSETTINGSDIALOG_H
#define ADVANCEDSETTINGSDIALOG_H

#include "constants.h"

#include <DDialog>

class AdvancedSettingsDialogPrivate;
class AdvancedSettingsDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit AdvancedSettingsDialog(QWidget *parent = nullptr);

    void setAdvancedSettings(const AdvancedSettings &settings);
    AdvancedSettings advancedSettings();

private:
    AdvancedSettingsDialogPrivate *const d;
};

#endif   // ADVANCEDSETTINGSDIALOG_H
