// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INLINECHATWIDGET_H
#define INLINECHATWIDGET_H

#include <QWidget>
#include "base/ai/abstractllm.h"

class InlineChatWidgetPrivate;
class InlineChatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit InlineChatWidget(QWidget *parent = nullptr);
    ~InlineChatWidget();
    void setLLM(AbstractLLM *llm);

public Q_SLOTS:
    void start();
    void reset();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    InlineChatWidgetPrivate *const d;
};

#endif   // INLINECHATWIDGET_H
