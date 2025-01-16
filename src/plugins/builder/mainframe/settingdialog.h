// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <DDialog>

#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>

class SettingDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);

    static QString defaultIssueFixPrompt();

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void initUI();
    void initConnection();
    void updateSettings();

    void handleAddPrompt();
    void handleDeletePrompt();
    void handleButtonClicked(int index);

private:
    QComboBox *promptCB { nullptr };
    QPushButton *addBtn { nullptr };
    QPushButton *delBtn { nullptr };
    QTextEdit *promptEdit { nullptr };
};

#endif   // SETTINGDIALOG_H
