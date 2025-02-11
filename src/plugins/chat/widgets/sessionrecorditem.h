// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONRECORDITEM_H
#define SESSIONRECORDITEM_H

#include "chatmanager.h"

#include <DWidget>
#include <DLabel>
#include <DToolButton>

class SessionRecordItem : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit SessionRecordItem(QWidget *parent = nullptr);

    void updateItem(const RecordData &data);

public Q_SLOTS:
    void onDeleteButtonClicked();
    void onRecordClicked();

Q_SIGNALS:
    void closeHistoryWidget();

private:
    void initUI();
    void initConnection();

    DTK_WIDGET_NAMESPACE::DToolButton *deleteButton { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *recordButton { nullptr };

    DTK_WIDGET_NAMESPACE::DLabel *promotLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *dateLabel { nullptr };

    QString talkId {};
};

#endif // SESSIONRECORDITEM_H
