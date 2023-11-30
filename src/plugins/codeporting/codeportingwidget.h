// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEPORTINGWIDGET_H
#define CODEPORTINGWIDGET_H

#include "common/widget/outputpane.h"

#include <DWidget>

class CodePortingWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit CodePortingWidget(QWidget *parent = nullptr);

private:
    void initUI();

    OutputPane *outputPane {nullptr};
    DTK_WIDGET_NAMESPACE::DWidget *reportPane {nullptr};
};

#endif // CODEPORTINGWIDGET_H
