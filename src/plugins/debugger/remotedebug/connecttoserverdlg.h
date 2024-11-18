// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONNECTTOSERVERDLG_H
#define CONNECTTOSERVERDLG_H

#include "services/debugger/debuggerservice.h"
#include "debuggerglobals.h"

#include <DDialog>
#include <DLineEdit>
#include <DTextEdit>
#include <DSuggestButton>

class ConnectToServerDlg : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit ConnectToServerDlg(QWidget *parent = nullptr);
    void setParam(const GdbserverInfo &info);

private slots:
    void on_pbtnOK_clicked();
    void on_pbtnCancel_clicked();

private:
    void setupUi();

    void setDirByFileDialog(Dtk::Widget::DLineEdit *edit);
    void setFileByFileDialog(Dtk::Widget::DLineEdit *edit);

    Dtk::Widget::DLineEdit *lePort = nullptr;
    Dtk::Widget::DLineEdit *leIP = nullptr;
    Dtk::Widget::DLineEdit *leDebugee = nullptr;
    Dtk::Widget::DLineEdit *leParam = nullptr;
    Dtk::Widget::DLineEdit *leProjectPath = nullptr;
    Dtk::Widget::DLineEdit *leDebugInfo = nullptr;
    Dtk::Widget::DTextEdit *leCommands = nullptr;

    Dtk::Widget::DSuggestButton *selDebugEE = nullptr;
    Dtk::Widget::DSuggestButton *selProject = nullptr;
    Dtk::Widget::DSuggestButton *selDebugInfo = nullptr;
};

#endif   // CONNECTTOSERVERDLG_H
