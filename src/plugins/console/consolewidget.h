// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include "qtermwidget.h" // 3drparty
#include "ColorScheme.h"

#include <QDir>

class ConsoleWidgetPrivate;
class ConsoleWidget : public QTermWidget
{
    Q_OBJECT
public:
    static ConsoleWidget *instance();
    explicit ConsoleWidget(QWidget *parent = nullptr);
    virtual ~ConsoleWidget();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    ConsoleWidgetPrivate *const d;
};

#endif // CONSOLEWIDGET_H
