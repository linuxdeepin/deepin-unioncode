// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MESSAGECOMPONENT_H
#define MESSAGECOMPONENT_H

#include "data/messagedata.h"

#include <QWidget>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QImage;
class QLabel;
class QVBoxLayout;
QT_END_NAMESPACE

class CodeEditComponent;
class MessageComponent : public QWidget
{
    enum UpdateState : uint8_t {
        Label,
        CodeEdit
    };
public:
    explicit MessageComponent(const MessageData &msgData, QWidget *parent = nullptr);
    void updateMessage(const MessageData &msgData);

private:
    void initUI();
    void initSenderInfo();
    void initMessageSection();

    void createCurrentUpdateWidget();

    QLabel *senderHead { nullptr };
    QLabel *senderName { nullptr };
    QLabel *context { nullptr };

    QVBoxLayout *msgLayout { nullptr };

    UpdateState currentUpdateState = Label;
    CodeEditComponent *curUpdateEdit { nullptr };
    QLabel *curUpdateLabel { nullptr };

    MessageData messageData;
};

#endif // MESSAGECOMPONENT_H
