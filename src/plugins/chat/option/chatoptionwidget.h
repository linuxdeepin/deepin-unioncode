// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHATOPTIONWIDGET_H
#define CHATOPTIONWIDGET_H

#include "common/common.h"

class ChatOptionWidgetPrivate;
class ChatOptionWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit ChatOptionWidget(QWidget *parent = nullptr);
    ~ChatOptionWidget() override;

    void saveConfig() override;
    void readConfig() override;

signals:

public slots:
private:
    ChatOptionWidgetPrivate *const d;
};

#endif // CHATOPTIONWIDGET_H
