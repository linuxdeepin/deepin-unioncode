// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "services/window/windowelement.h"
#include <DMainWindow>
#include <DMenu>

#include <QObject>
#include <QWidgetList>
#include <QMap>
#include <QString>

DWIDGET_USE_NAMESPACE
using dpfservice::Position;
class MainWindowPrivate;
class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //dockWidget
    void addWidget(const QString &name, QWidget *widget, Position pos = Position::FullWindow);

    void replaceWidget(const QString &name, QWidget *widget, Position pos = Position::FullWindow);

    void hideWidget(const QString &name);
    void hideWidget(Position pos);
    void hideAllWidget();

    void showWidget(const QString &name);
    void showWidget(Position pos);
    void showAllWidget();

    void removeWidget(const QString &name);
    void removeWidget(Position pos);

    //only work for dockWidget,can`t work for centralWidget
    void splitWidgetOrientation(const QString &first, const QString &second, Qt::Orientation orientation);

    //toolbar
    void setToolbar(Qt::ToolBarArea area, QWidget *widget);

    //topToolBar - titleBar
    void clearTopTollBar();
    void hideTopTollBar();
    void addTopToolItem(const QString &name, QAction *action);
    void showTopToolItem(const QString &name);
    void addTopToolBarSpacing(int spacing);

    static Qt::DockWidgetArea positionTodockArea(Position pos);

private:
    MainWindowPrivate *d;

    DIconButton *createIconButton(QAction *action);
    void addTopToolBar();
    void removeAllDockWidget();
};
Q_DECLARE_METATYPE(Position)

#endif   // MAINWINDOW_H
