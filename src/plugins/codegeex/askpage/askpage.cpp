// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "askpage.h"
#include "common/util/custompaths.h"

#include <QtCore/QVariant>
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QScrollBar>
#include <QUuid>
#include <QSysInfo>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

// TODO(mozart):get those variable from config pane.
static const char *kUrlSSEChat = "https://codegeex.cn/prod/code/chatGlmSse/chat";
static const char *kUrlNewSession = "https://codegeex.cn/prod/code/chatGlmTalk/insert";

using namespace CodeGeeX;
AskPage::AskPage(QWidget *parent) : QWidget(parent)
  , timer(new QTimer(this))
{
    setupUi(this);

    connect(&askApi, &AskApi::loginState, [this](AskApi::LoginState loginState){
        if (loginState == AskApi::LoginState::kLoginFailed) {
            loginSuccess = false;
            this->label->setText("login failed");
            // switch to login ui.
        } else if (loginState == AskApi::LoginState::kLoginSuccess) {
            loginSuccess = true;
            this->label->setText("login success");
            // switch to ask page.
        }
    });

    loadConfig();

    connect(timer, SIGNAL(timeout()), this, SLOT(queryLoginState()));
    timer->start(1000);

    connect(&askApi, &AskApi::response, [this](const QString &response, const QString &event){
        int responseSize = response.size();
        if (responseSize < totalResponseSize || responseSize == 0)
            return;

        if (event == "finish") {
            totalResponseSize = 0;
        } else if (event == "add"){
            totalResponseSize = responseSize;
        }

        outputpage->setPlainText(response);

        // scroll to bottom.
        outputpage->verticalScrollBar()->setValue(outputpage->verticalScrollBar()->maximum());
    });
}


QString uuid()
{
    QUuid uuid = QUuid::createUuid();
    return uuid.toString().replace("{", "").replace("}", "").replace("-", "");;
}

void AskPage::on_btnLogin_clicked()
{
    if (sessionId.isEmpty() || userId.isEmpty()) {
        sessionId = uuid();
        userId = uuid();
        saveConfig(sessionId, userId);
    }
    QString machineId = QSysInfo::machineUniqueId();
    askApi.sendLoginRequest(sessionId, machineId, userId);
}

void AskPage::on_btnDelete_clicked()
{

}

void AskPage::on_btnHistory_clicked()
{

}

void AskPage::on_btnNewSession_clicked()
{
    QString prompt("title");
    QString taskid(uuid());
    askApi.postNewSession(kUrlNewSession, sessionId, prompt, taskid);
}

void AskPage::on_btnSend_clicked()
{
    QString prompt = lineEditInput->text();
    QMultiMap<QString, QString> history;
    QString machineId = QSysInfo::machineUniqueId();
    askApi.postSSEChat(kUrlSSEChat, sessionId, prompt, machineId, history);
}

void AskPage::setupUi(QWidget *AskPage)
{
    if (AskPage->objectName().isEmpty())
        AskPage->setObjectName(QStringLiteral("AskPage"));
    AskPage->resize(400, 300);
    verticalLayout = new QVBoxLayout(AskPage);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    toolLayout = new QHBoxLayout();
    toolLayout->setSpacing(6);
    toolLayout->setObjectName(QStringLiteral("toolLayout"));
    btnLogin = new QPushButton(AskPage);
    btnLogin->setObjectName(QStringLiteral("btnLogin"));

    toolLayout->addWidget(btnLogin);

    btnDelete = new QPushButton(AskPage);
    btnDelete->setObjectName(QStringLiteral("btnDelete"));

    toolLayout->addWidget(btnDelete);

    btnHistory = new QPushButton(AskPage);
    btnHistory->setObjectName(QStringLiteral("btnHistory"));

    toolLayout->addWidget(btnHistory);

    btnNewSession = new QPushButton(AskPage);
    btnNewSession->setObjectName(QStringLiteral("btnNewSession"));

    toolLayout->addWidget(btnNewSession);

    verticalLayout->addLayout(toolLayout);

    label = new QLabel(AskPage);
    label->setObjectName(QStringLiteral("label"));

    verticalLayout->addWidget(label);

    outputpage = new QPlainTextEdit(AskPage);
    outputpage->setObjectName(QStringLiteral("outputpage"));

    verticalLayout->addWidget(outputpage);

    inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(6);
    inputLayout->setObjectName(QStringLiteral("inputLayout"));
    lineEditInput = new QLineEdit(AskPage);
    lineEditInput->setObjectName(QStringLiteral("lineEditInput"));

    QObject::connect(lineEditInput, &QLineEdit::returnPressed, [=]() {
        on_btnSend_clicked();
        lineEditInput->clear();
    });

    inputLayout->addWidget(lineEditInput);

    btnSend = new QPushButton(AskPage);
    btnSend->setObjectName(QStringLiteral("btnSend"));

    inputLayout->addWidget(btnSend);

    verticalLayout->addLayout(inputLayout);

    AskPage->setWindowTitle(QApplication::translate("AskPage", "AskPage", nullptr));
    btnLogin->setText(QApplication::translate("AskPage", "login", nullptr));
    btnDelete->setText(QApplication::translate("AskPage", "delete", nullptr));
    btnHistory->setText(QApplication::translate("AskPage", "history", nullptr));
    btnNewSession->setText(QApplication::translate("AskPage", "new session", nullptr));
    label->setText(QApplication::translate("AskPage", "TextLabel", nullptr));
    btnSend->setText(QApplication::translate("AskPage", "Send", nullptr));

    QMetaObject::connectSlotsByName(AskPage);
}

void AskPage::saveConfig(const QString &sessionId, const QString &userId)
{
    QJsonObject config;
    config["sessionId"] = sessionId;
    config["userId"] = userId;

    QJsonDocument document(config);

    QFile file(configFilePath());
    file.open(QIODevice::WriteOnly);
    file.write(document.toJson());
    file.close();
}

void AskPage::loadConfig()
{
    QFile file(configFilePath());
    if (!file.exists())
        return;

    file.open(QIODevice::ReadOnly);
    QString data = QString::fromUtf8(file.readAll());
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject config = document.object();
    if (!config.empty()) {
        sessionId = config["sessionId"].toString();
        userId = config["userId"].toString();
    }
}

QString AskPage::configFilePath() const
{
    return CustomPaths::user(CustomPaths::Configures) + "/codegeexcfg.json";
}

void AskPage::queryLoginState()
{
    if (!sessionId.isEmpty()) {
        askApi.sendQueryRequest(sessionId);
    }
}
