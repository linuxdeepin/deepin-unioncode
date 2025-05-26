// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GENERALSETTINGWIDGET_H
#define GENERALSETTINGWIDGET_H

#include <DFrame>
#include <DComboBox>
#include <DPushButton>
#include <DLineEdit>

class GeneralSettingWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit GeneralSettingWidget(QWidget *parent = nullptr);

    bool apply();
    void updateSettings();

private:
    void initUI();
    void initConnection();

private Q_SLOTS:
    void handleTemplateChanged();
    void handleAddTemplate();
    void handleDeleteTemplate();

private:
    DTK_WIDGET_NAMESPACE::DComboBox *testFrameworkCB { nullptr };
    DTK_WIDGET_NAMESPACE::DComboBox *templateCB { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *templateAddBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *templateDelBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DLineEdit *nameFormatEdit { nullptr };
};

#endif   // GENERALSETTINGWIDGET_H
