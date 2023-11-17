// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEGEEXWIDGET_H
#define CODEGEEXWIDGET_H

#include <DWidget>
#include <DTabBar>
#include <DPushButton>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QTabBar;
class QStackedWidget;
class QPushButton;
QT_END_NAMESPACE

class AskPageWidget;
class TranslationPageWidget;
class CodeGeeXWidget : public DWidget
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

private:
    void initUI();
    void initConnection();

    void initAskWidget();
    void initTabBar();
    void initStackWidget();
    void initAskWidgetConnection();

    void resetHeaderBtns();

    DTabBar *tabBar { nullptr };
    DStackedWidget *stackWidget { nullptr };
    DPushButton *deleteBtn { nullptr };
    DPushButton *historyBtn { nullptr };
    DPushButton *createNewBtn { nullptr };

    AskPageWidget *askPage { nullptr };
    TranslationPageWidget *transPage { nullptr };

    CurrentPageState currentState { AskPage };
};

#endif // CODEGEEXWIDGET_H
