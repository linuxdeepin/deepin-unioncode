// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef ASKPAGE_H
#define ASKPAGE_H

#include "codegeex/askapi.h"

#include <DWidget>
#include <DLabel>
#include <DLineEdit>

DWIDGET_USE_NAMESPACE

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QPlainTextEdit;
class QLineEdit;
class QTimer;
class AskPage : public DWidget
{
    Q_OBJECT
public:
    explicit AskPage(QWidget *parent = nullptr);

signals:

private slots:
    void on_btnLogin_clicked();

    void on_btnDelete_clicked();

    void on_btnHistory_clicked();

    void on_btnNewSession_clicked();

    void on_btnSend_clicked();

    void queryLoginState();

private:
    void setupUi(DWidget *AskPage);
    void saveConfig(const QString &sessionId, const QString &userId);
    void loadConfig();
    QString configFilePath() const;

    QVBoxLayout *verticalLayout = nullptr;
    QHBoxLayout *toolLayout = nullptr;
    DPushButton *btnLogin = nullptr;
    DPushButton *btnDelete = nullptr;
    DPushButton *btnHistory = nullptr;
    DPushButton *btnNewSession = nullptr;
    DLabel *label = nullptr;
    DPlainTextEdit *outputpage = nullptr;
    QHBoxLayout *inputLayout = nullptr;
    DLineEdit *lineEditInput = nullptr;
    DPushButton *btnSend = nullptr;

    CodeGeeX::AskApi askApi;
    QString sessionId;
    QString userId;
    QTimer *timer = nullptr;
    bool loginSuccess = false;
    int totalResponseSize = 0;
};

#endif // ASKPAGE_H
