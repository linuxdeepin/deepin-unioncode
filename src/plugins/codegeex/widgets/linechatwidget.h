// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINECHATWIDGET_H
#define LINECHATWIDGET_H

#include <QWidget>

class LineChatWidgetPrivate;
class LineChatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LineChatWidget(QWidget *parent = nullptr);
    ~LineChatWidget();

    void showLineChat();

protected:
    void showEvent(QShowEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void hideEvent(QHideEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    LineChatWidgetPrivate *const d;
};

#endif // LINECHATWIDGET_H
