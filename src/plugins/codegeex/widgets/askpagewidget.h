// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ASKPAGEWIDGET_H
#define ASKPAGEWIDGET_H

#include "data/messagedata.h"

#include <QMap>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QScrollArea;
class QPushButton;
class QLineEdit;
QT_END_NAMESPACE

class MessageComponent;
class AskPageWidget : public QWidget
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

private:
    void initUI();
    void initInputWidget();
    void initConnection();

    void cleanWidgets();
    void setSessionPage();

    PageState curState;
    QScrollArea *scrollArea { nullptr };
    QWidget *inputWidget { nullptr };
    QWidget *messageContainer { nullptr };
    QPushButton *sendBtn { nullptr };
    QLineEdit *inputEdit { nullptr };

    QMap<QString, MessageComponent*> msgComponents {};
};

#endif // ASKPAGEWIDGET_H
