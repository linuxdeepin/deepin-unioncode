// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ASKPAGEWIDGET_H
#define ASKPAGEWIDGET_H

#include "data/messagedata.h"

#include <DWidget>
#include <DLineEdit>
#include <DFloatingButton>
#include <DTextEdit>

#include <QMap>

QT_BEGIN_NAMESPACE
class QScrollArea;
class QPushButton;
class QLineEdit;
QT_END_NAMESPACE

class InputEdit : public DTK_WIDGET_NAMESPACE::DTextEdit
{
    Q_OBJECT
public:
    explicit InputEdit(QWidget *parent = nullptr);

signals:
    void pressedEnter();

protected:
    void keyPressEvent(QKeyEvent *e) override;
};

class MessageComponent;
class AskPageWidget : public DTK_WIDGET_NAMESPACE::DWidget
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
    void introPageShown();
    void sessionPageShown();
    void requestShowHistoryPage();

public Q_SLOTS:
    void onMessageUpdate(const MessageData &msgData);
    void slotMessageSend();
    void onChatFinished();
    void onDeleteBtnClicked();
    void onHistoryBtnClicked();
    void onCreateNewBtnClicked();
    void setInputText(const QString &prompt);

private:
    void initUI();
    void initInputWidget();
    void initConnection();

    void cleanWidgets();
    void setSessionPage();

    void enterAnswerState();
    void enterInputState();
    void waitForAnswer();

    void askQuestion(const QString &question);
    void resetBtns();

    DTK_WIDGET_NAMESPACE::DScrollArea *scrollArea { nullptr };
    DTK_WIDGET_NAMESPACE::DWidget *inputWidget { nullptr };
    DTK_WIDGET_NAMESPACE::DWidget *messageContainer { nullptr };
    InputEdit *inputEdit { nullptr };

    DTK_WIDGET_NAMESPACE::DFloatingButton *sendButton { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *deleteBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *historyBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *createNewBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DWidget *stopWidget { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *stopGenerate { nullptr };

    MessageComponent *waitComponets { nullptr };
    QMap<QString, MessageComponent *> msgComponents {};

    QString placeHolderText {};
    int progressCalcNum = 0;
    PageState curState;
    bool waitingAnswer { false };
};

#endif   // ASKPAGEWIDGET_H
