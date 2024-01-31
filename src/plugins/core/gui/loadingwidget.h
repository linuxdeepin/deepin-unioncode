// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <DWidget>
#include <DLabel>

#include <QTimer>
#include <QGraphicsOpacityEffect>

class loadingWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit loadingWidget(QWidget *parent = nullptr);

private:
    void setLogo();

    DTK_WIDGET_NAMESPACE::DLabel *logo { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *backgroundLogo { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *loadingText { nullptr };
    QGraphicsOpacityEffect *opacityEffect { nullptr };
    QTimer timer;
};

#endif   // LOADINGWIDGET_H
