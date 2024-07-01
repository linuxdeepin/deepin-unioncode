// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINFRAME_H
#define MAINFRAME_H

#include "common/widget/appoutputpane.h"

#include <DFrame>

class MainFramePrivate;
class MainFrame : public DTK_WIDGET_NAMESPACE::DFrame
{
public:
    MainFrame(QWidget *parent = nullptr);
    ~MainFrame();
    static bool checkToolInstalled(const QString &tool);

private slots:
    void runApp(const QString &appId);
    void installApp(const QString &appId);
    void uninstallApp(const QString &appId);

private:
    MainFramePrivate *d { nullptr };

    void initUi();
    void initConnection();
    void initTable();
    void updateInstalled();
    void updateRepository(const QString &text);
    void updateRunning();

    void output(const QString &text, OutputPane::OutputFormat format = OutputPane::NormalMessage);
};

#endif   // MAINFRAME_H
