// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "services/window/windowcontroller.h"
#include "gui/dockheader.h"

#include <DMainWindow>
#include <DMenu>
#include <DDockWidget>

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
    void addWidget(const QString &name, QWidget *widget, Position pos, Qt::Orientation orientation);

    void replaceWidget(const QString &name, QWidget *widget, Position pos = Position::FullWindow);

    void hideWidget(const QString &name);
    void hideWidget(Position pos);
    void hideAllWidget();

    void showWidget(const QString &name);
    void showWidget(Position pos);
    void showAllWidget();

    void removeWidget(const QString &name);
    void removeWidget(Position pos);

    QString getCurrentDockName(Position pos);

    void deleteDockHeader(const QString &name);
    void addToolBtnToDockHeader(const QString &dockName, DToolButton *btn);
    void setDockWidgetFeatures(const QString &name, QDockWidget::DockWidgetFeatures feature);
    //size(width,heigt) width used for widget in Left and Right.
    //heigth used for widget in top and bottom
    void resizeDock(const QString &dockName, QSize size);
    //only work for dockWidget,can`t work for centralWidget
    void splitWidgetOrientation(const QString &first, const QString &second, Qt::Orientation orientation);

    //toolbar
    void setToolbar(Qt::ToolBarArea area, QWidget *widget);

    //topToolBar - titleBar
    void showTopToolBar();
    void hideTopTollBar();
    void setLeftTopToolWidget(DWidget *widget);
    void setMiddleTopToolWidget(DWidget *widget);
    void setRightTopToolWidget(DWidget *widget);

    static Qt::DockWidgetArea positionTodockArea(Position pos);

private:
    MainWindowPrivate *d;

    DDockWidget *createDockWidget(DWidget *widget);
    void initDockHeader(DDockWidget *dock, Position pos);
    void addTopToolBar();
    void removeAllDockWidget();

    void resizeDock(QDockWidget *dock);
};
Q_DECLARE_METATYPE(Position)

#endif   // MAINWINDOW_H
