// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MESSAGECOMPONENT_H
#define MESSAGECOMPONENT_H

#include "data/messagedata.h"
#include "codegeexmanager.h"

#include <DWidget>
#include <DLabel>
#include <DFrame>
#include <DSpinner>

#include <QVariant>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QImage;
class QLabel;
class QVBoxLayout;
QT_END_NAMESPACE

class CodeEditComponent;
class MessageComponent : public DFrame
{
    Q_OBJECT
    enum UpdateState : uint8_t {
        Label,
        CodeEdit
    };

public:
    explicit MessageComponent(const MessageData &msgData, QWidget *parent = nullptr);
    void updateMessage(const MessageData &msgData);
    void waitForAnswer();
    void stopWaiting();

private:
    void initUI();
    void initSenderInfo();
    void initMessageSection();
    void initConnect();

    void showWebsitesRefrences();
    void createCurrentUpdateWidget();
    bool createCodeEdit(const MessageData &newData);

    bool waitingAnswer { false };
    bool isConnecting { false };
    bool finished { false };

    DLabel *senderHead { nullptr };
    DLabel *senderName { nullptr };
    DLabel *context { nullptr };
    DPushButton *editButton { nullptr };
    DWidget *searchingWidget { nullptr };

    QVBoxLayout *msgLayout { nullptr };

    CodeEditComponent *curUpdateEdit { nullptr };
    DLabel *curUpdateLabel { nullptr };

    MessageData messageData;
    UpdateState currentUpdateState = Label;

    QList<CodeGeeX::websiteReference> websites;
};

#endif   // MESSAGECOMPONENT_H
