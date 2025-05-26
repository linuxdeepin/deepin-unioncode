// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class JsonTabWidget;
class MainWindowPrivate;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    MainWindowPrivate *const d;
public:
    explicit MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~MainWindow();
    void start(uint pid);
    void stop();
};

#endif // MAINWINDOW_H
