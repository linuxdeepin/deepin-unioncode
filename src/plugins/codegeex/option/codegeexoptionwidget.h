// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEGEEXOPTIONWIDGET_H
#define CODEGEEXOPTIONWIDGET_H

#include "common/common.h"

class CodeGeeXOptionWidgetPrivate;
class CodeGeeXOptionWidget : public PageWidget
{
    Q_OBJECT
public:
    explicit CodeGeeXOptionWidget(QWidget *parent = nullptr);
    ~CodeGeeXOptionWidget() override;

    void saveConfig() override;
    void readConfig() override;

signals:

public slots:
private:
    CodeGeeXOptionWidgetPrivate *const d;
};

#endif // CODEGEEXOPTIONWIDGET_H
