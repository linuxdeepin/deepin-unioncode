// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEGEEXWIDGET_H
#define CODEGEEXWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTabBar;
class QStackedWidget;
class QPushButton;
QT_END_NAMESPACE

class AskPageWidget;
class TranslationPageWidget;
class CodeGeeXWidget : public QWidget
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
    void onDeleteBtnClicked();
    void onHistoryBtnClicked();
    void onCreateNewBtnClicked();

private:
    void initUI();
    void initLoginConnection();

    void initAskWidget();
    void initTabBar();
    void initStackWidget();
    void initAskWidgetConnection();

    void resetHeaderBtns();

    QPushButton *loginBtn { nullptr };
    QTabBar *tabBar { nullptr };
    QStackedWidget *stackWidget { nullptr };
    QPushButton *deleteBtn { nullptr };
    QPushButton *historyBtn { nullptr };
    QPushButton *createNewBtn { nullptr };

    AskPageWidget *askPage { nullptr };
    TranslationPageWidget *transPage { nullptr };

    CurrentPageState currentState { AskPage };
};

#endif // CODEGEEXWIDGET_H
