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

class CodeGeeXWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CodeGeeXWidget(QWidget* parent = nullptr);

public Q_SLOTS:
    void onLoginSuccessed();

private:
    void initUI();
    void initLoginConnection();

    void initAskWidget();
    void initTabBar();
    void initStackWidget();
    void initAskWidgetConnection();

    QPushButton *loginBtn { nullptr };
    QTabBar *tabBar { nullptr };
    QStackedWidget *stackWidget { nullptr };
};

#endif // CODEGEEXWIDGET_H
