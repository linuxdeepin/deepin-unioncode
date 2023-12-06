// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MESSAGECOMPONENT_H
#define MESSAGECOMPONENT_H

#include "data/messagedata.h"

#include <DWidget>
#include <DLabel>
#include <DFrame>

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
    bool createCodeEdit(const MessageData &newData);

    DLabel *senderHead { nullptr };
    DLabel *senderName { nullptr };
    DLabel *context { nullptr };

    QVBoxLayout *msgLayout { nullptr };

    CodeEditComponent *curUpdateEdit { nullptr };
    DLabel *curUpdateLabel { nullptr };

    MessageData messageData;
    UpdateState currentUpdateState = Label;
};

#endif // MESSAGECOMPONENT_H
