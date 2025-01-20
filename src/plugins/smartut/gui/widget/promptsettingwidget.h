// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROMPTSETTINGWIDGET_H
#define PROMPTSETTINGWIDGET_H

#include <DFrame>
#include <DComboBox>
#include <DPushButton>

#include <QTextEdit>

class PromptSettingWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit PromptSettingWidget(QWidget *parent = nullptr);

    void apply();
    void updateSettings();

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void initUI();
    void initConnection();

    void handleAddPrompt();
    void handleDeletePrompt();

private:
    DTK_WIDGET_NAMESPACE::DComboBox *promptCB { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *addBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *delBtn { nullptr };
    QTextEdit *promptEdit { nullptr };
};

#endif   // PROMPTSETTINGWIDGET_H
