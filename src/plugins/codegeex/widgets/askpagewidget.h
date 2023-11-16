// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ASKPAGEWIDGET_H
#define ASKPAGEWIDGET_H

#include "data/messagedata.h"

#include <DWidget>
#include <DLineEdit>

#include <QMap>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QScrollArea;
class QPushButton;
class QLineEdit;
QT_END_NAMESPACE

class MessageComponent;
class AskPageWidget : public DWidget
{
    Q_OBJECT
public:
    enum PageState : uint8_t {
        Intro,
        Session
    };

    explicit AskPageWidget(QWidget *parent = nullptr);
    void setIntroPage();
    bool isIntroPageState();

Q_SIGNALS:
    void sendPromot(const QString &promot);
    void introPageShown();
    void sessionPageShown();

public Q_SLOTS:
    void onMessageUpdate(const MessageData &msgData);
    void onSendBtnClicked();
    void onChatFinished();

private:
    void initUI();
    void initInputWidget();
    void initConnection();

    void cleanWidgets();
    void setSessionPage();

    void enterAnswerState();
    void enterInputState();

    void askQuestion(const QString &question);

    PageState curState;

    DScrollArea *scrollArea { nullptr };
    DWidget *inputWidget { nullptr };
    DWidget *messageContainer { nullptr };
    DLineEdit *inputEdit { nullptr };
    QTimer *processTimer { nullptr };
    int progressCalcNum = 0;
    QString placeHolderText;

    QMap<QString, MessageComponent*> msgComponents {};
};

#endif // ASKPAGEWIDGET_H
