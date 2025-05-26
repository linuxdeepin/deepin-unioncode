// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef RRDIALOG_H
#define RRDIALOG_H

#include "services/language/languageservice.h"

#include <DDialog>
#include <DLineEdit>

class RecordDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT

public:
    explicit RecordDialog(QWidget *parent = nullptr);
    ~RecordDialog();

private slots:
    void on_pbtnOK_clicked();

    void on_pbtnCancel_clicked();

signals:
    void startRecord(const dpfservice::RunCommandInfo &args);

private:
    void setupUi();
};

class ReplayDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT

public:
    explicit ReplayDialog(QWidget *parent = nullptr);
    ~ReplayDialog();

signals:
    void startReplay(const QString &target);

private slots:
    void on_pbtnOK_clicked();

    void on_pbtnCancel_clicked();

    void showFileDialog();

private:
    void setupUi();

    Dtk::Widget::DLineEdit *traceDir = nullptr;
};

#endif   // RRDIALOG_H
