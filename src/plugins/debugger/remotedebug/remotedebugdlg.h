// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef REMOTEDEBUGDLG_H
#define REMOTEDEBUGDLG_H

#include <DDialog>
#include <DLineEdit>

class RemoteDebugDlg : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT

public:
    explicit RemoteDebugDlg(QWidget *parent = nullptr);
    ~RemoteDebugDlg();

private slots:
    void on_pbtnOK_clicked();

    void on_pbtnCancel_clicked();

private:
    void setupUi();

    Dtk::Widget::DLineEdit *lePort = nullptr;
    Dtk::Widget::DLineEdit *leIP = nullptr;
    Dtk::Widget::DLineEdit *leDebugee = nullptr;
    Dtk::Widget::DLineEdit *leProjectPath = nullptr;
    Dtk::Widget::DLineEdit *leParameters = nullptr;
};

#endif // REMOTEDEBUGDLG_H
