// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef ASKPAGE_H
#define ASKPAGE_H

#include "codegeex/askapi.h"

#include <QWidget>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QPlainTextEdit;
class QLineEdit;
class QTimer;
class AskPage : public QWidget
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
    void setupUi(QWidget *AskPage);
    void saveConfig(const QString &sessionId, const QString &userId);
    void loadConfig();
    QString configFilePath() const;

    QVBoxLayout *verticalLayout = nullptr;
    QHBoxLayout *toolLayout = nullptr;
    QPushButton *btnLogin = nullptr;
    QPushButton *btnDelete = nullptr;
    QPushButton *btnHistory = nullptr;
    QPushButton *btnNewSession = nullptr;
    QLabel *label = nullptr;
    QPlainTextEdit *outputpage = nullptr;
    QHBoxLayout *inputLayout = nullptr;
    QLineEdit *lineEditInput = nullptr;
    QPushButton *btnSend = nullptr;

    CodeGeeX::AskApi askApi;
    QString sessionId;
    QString userId;
    QTimer *timer = nullptr;
    bool loginSuccess = false;
    int totalResponseSize = 0;
};

#endif // ASKPAGE_H
