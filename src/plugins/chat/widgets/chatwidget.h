// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <DFrame>
#include <DPushButton>
#include <DButtonBox>

#include <QPropertyAnimation>

class AskPageWidget;
class HistoryListWidget;
class ChatWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT
public:
    explicit ChatWidget(QWidget *parent = nullptr);

public Q_SLOTS:
    void onNewSessionCreated();
    void onCloseHistoryWidget();
    void onShowHistoryWidget();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void initUI();
    void initConnection();

    void initAskWidget();
    void initHistoryWidget();
    void initStackWidget();
    void initAskWidgetConnection();
    void initHistoryWidgetConnection();

    DTK_WIDGET_NAMESPACE::DStackedWidget *stackWidget { nullptr };

    QPropertyAnimation *historyWidgetAnimation { nullptr };

    AskPageWidget *askPage { nullptr };
    HistoryListWidget *historyWidget { nullptr };

    bool historyShowed { false };
};

#endif   // CHAT_WIDGET_H
