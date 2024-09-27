// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INLINECHATWIDGET_H
#define INLINECHATWIDGET_H

#include <QWidget>

class InlineChatWidgetPrivate;
class InlineChatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit InlineChatWidget(QWidget *parent = nullptr);
    ~InlineChatWidget();

    void start();

protected:
    void showEvent(QShowEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void hideEvent(QHideEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    InlineChatWidgetPrivate *const d;
};

#endif // INLINECHATWIDGET_H
