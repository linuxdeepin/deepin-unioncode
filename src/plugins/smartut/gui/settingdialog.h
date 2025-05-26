// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include "widget/generalsettingwidget.h"
#include "widget/promptsettingwidget.h"
#include "widget/resourcesettingwidget.h"

#include <DDialog>
#include <DButtonBox>

#include <QStackedWidget>

class SettingDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);

    QStringList selectedFileList() const;
    QString selectedProject() const;
    QString targetLocation() const;

protected:
    void showEvent(QShowEvent *e) override;

private:
    void initUI();
    void initConnection();

    void handleSwitchWidget(QAbstractButton *btn);
    void handleButtonClicked();

    GeneralSettingWidget *generalWidget { nullptr };
    PromptSettingWidget *promptWidget { nullptr };
    ResourceSettingWidget *resourceWidget { nullptr };
    DTK_WIDGET_NAMESPACE::DButtonBox *btnBox { nullptr };
    QStackedWidget *mainWidget { nullptr };
    QPushButton *cancelBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DSuggestButton *okBtn { nullptr };
};

#endif   // SETTINGDIALOG_H
