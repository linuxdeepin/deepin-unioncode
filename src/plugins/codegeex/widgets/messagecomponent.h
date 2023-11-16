// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MESSAGECOMPONENT_H
#define MESSAGECOMPONENT_H

#include "data/messagedata.h"

#include <DWidget>
#include <DLabel>

#include <QVariant>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QImage;
class QLabel;
class QVBoxLayout;
QT_END_NAMESPACE

class CodeEditComponent;
class MessageComponent : public DWidget
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

    UpdateState currentUpdateState = Label;
    CodeEditComponent *curUpdateEdit { nullptr };
    DLabel *curUpdateLabel { nullptr };

    MessageData messageData;
};

#endif // MESSAGECOMPONENT_H
