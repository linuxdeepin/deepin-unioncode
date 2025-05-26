// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HISTORYLISTWIDGET_H
#define HISTORYLISTWIDGET_H

#include "chatmanager.h"

#include <DWidget>
#include <DPushButton>

DWIDGET_USE_NAMESPACE

class SessionRecordItem;
class PageControlComponent;
class HistoryListWidget : public DWidget
{
    Q_OBJECT
public:
    explicit HistoryListWidget(QWidget* parent = nullptr);

public Q_SLOTS:
    void onSessionRecordUpdated();
    void onPageChanged(int index);

Q_SIGNALS:
    void requestCloseHistoryWidget();

private:
    void initUI();
    void initConnection();

    void updateItems(const QList<RecordData> &dataList);

    DPushButton *closeButton { nullptr };
    PageControlComponent *pageController { nullptr };
    QList<SessionRecordItem *> itemsList {};
};

#endif // HISTORYLISTWIDGET_H
