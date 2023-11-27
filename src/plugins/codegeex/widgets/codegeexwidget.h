// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEGEEXWIDGET_H
#define CODEGEEXWIDGET_H

#include <DWidget>
#include <DTabBar>
#include <DPushButton>

#include <QPropertyAnimation>

class AskPageWidget;
class HistoryListWidget;
class TranslationPageWidget;
class CodeGeeXWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT

    enum CurrentPageState : uint8_t {
        AskPage,
        TrasnlatePage
    };

public:
    explicit CodeGeeXWidget(QWidget* parent = nullptr);

public Q_SLOTS:
    void onLoginSuccessed();
    void onNewSessionCreated();
    void onDeleteBtnClicked();
    void onHistoryBtnClicked();
    void onCreateNewBtnClicked();
    void toTranslateCode(const QString &code);
    void onAnwserFinished();
    void onAnwserStarted();
    void onCloseHistoryWidget();

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

    void resetHeaderBtns();

    DTK_WIDGET_NAMESPACE::DTabBar *tabBar { nullptr };
    DTK_WIDGET_NAMESPACE::DStackedWidget *stackWidget { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *deleteBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *historyBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *createNewBtn { nullptr };

    QPropertyAnimation *historyWidgetAnimation { nullptr };

    AskPageWidget *askPage { nullptr };
    HistoryListWidget *historyWidget { nullptr };
    TranslationPageWidget *transPage { nullptr };

    CurrentPageState currentState { AskPage };
    bool historyShowed { false };
};

#endif // CODEGEEXWIDGET_H
