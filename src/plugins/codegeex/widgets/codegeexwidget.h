// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEGEEXWIDGET_H
#define CODEGEEXWIDGET_H

#include <DFrame>
#include <DPushButton>
#include <DButtonBox>

#include <QPropertyAnimation>

class AskPageWidget;
class HistoryListWidget;
class TranslationPageWidget;
class CodeGeeXWidget : public DTK_WIDGET_NAMESPACE::DFrame
{
    Q_OBJECT

    enum CurrentPageState : uint8_t {
        AskPage,
        TrasnlatePage
    };

public:
    explicit CodeGeeXWidget(QWidget *parent = nullptr);

public Q_SLOTS:
    void onLoginSuccessed();
    void onLogOut();
    void onNewSessionCreated();
    void toTranslateCode(const QString &code);
    void onCloseHistoryWidget();
    void onShowHistoryWidget();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void initUI();
    void initConnection();

    void initAskWidget();
    void initHistoryWidget();
    void initTabBar();
    void initStackWidget();
    void initAskWidgetConnection();
    void initHistoryWidgetConnection();

    DTK_WIDGET_NAMESPACE::DButtonBox *tabBar { nullptr };
    DTK_WIDGET_NAMESPACE::DStackedWidget *stackWidget { nullptr };

    QPropertyAnimation *historyWidgetAnimation { nullptr };

    AskPageWidget *askPage { nullptr };
    HistoryListWidget *historyWidget { nullptr };
    TranslationPageWidget *transPage { nullptr };

    bool historyShowed { false };
};

#endif   // CODEGEEXWIDGET_H
