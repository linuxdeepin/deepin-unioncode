// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include "qtermwidget.h" // 3drparty
#include "ColorScheme.h"

#include <DGuiApplicationHelper>

#include <QDir>
DGUI_USE_NAMESPACE

class ConsoleWidgetPrivate;
class ConsoleWidget : public QTermWidget
{
    Q_OBJECT
public:
    explicit ConsoleWidget(QWidget *parent = nullptr, bool startNow = true);
    virtual ~ConsoleWidget();

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void updateColorScheme(DGuiApplicationHelper::ColorType themetype);
private:
    ConsoleWidgetPrivate *const d;

    void enterCurrentProjectPath();
};

#endif // CONSOLEWIDGET_H
