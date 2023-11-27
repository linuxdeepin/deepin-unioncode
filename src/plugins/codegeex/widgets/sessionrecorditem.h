// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONRECORDITEM_H
#define SESSIONRECORDITEM_H

#include "codegeexmanager.h"

#include <DWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class SessionRecordItem : public DWidget
{
    Q_OBJECT
public:
    explicit SessionRecordItem(QWidget *parent = nullptr);

    void updateItem(const RecordData &data);

public Q_SLOTS:
    void onDeleteButtonClicked();
    void onRecordClicked();

private:
    void initUI();
    void initConnection();

    DPushButton *deleteButton { nullptr };
    DPushButton *recordButton { nullptr };

    DLabel *promotLabel { nullptr };
    DLabel *dateLabel { nullptr };

    QString talkId {};
};

#endif // SESSIONRECORDITEM_H
