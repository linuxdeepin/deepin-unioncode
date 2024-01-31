// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWSTATUSBAR_H
#define WINDOWSTATUSBAR_H

#include <DStatusBar>

class WindowStatusBarPrivate;
class WindowStatusBar : public DTK_WIDGET_NAMESPACE::DStatusBar
{
    Q_OBJECT
    WindowStatusBarPrivate *const d;
public:
    explicit WindowStatusBar(QWidget *parent = nullptr);
    virtual ~WindowStatusBar();
    void setPercentage(int percentage);
    void setMessage(const QString &message);
    bool progressIsHidden();
    void hideProgress();
    void showProgress();
};

#endif // WINDOWSTATUSBAR_H
