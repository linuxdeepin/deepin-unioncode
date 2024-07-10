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
    QDockWidget *addWidget(const QString &name, QWidget *widget, Position pos = Position::FullWindow);
    QDockWidget *addWidget(const QString &name, QWidget *widget, Position pos, Qt::Orientation orientation);

    void replaceWidget(const QString &name, QWidget *widget, Position pos = Position::FullWindow);

    void hideWidget(const QString &name);
    void hideWidget(Position pos);
    void hideAllWidget();

    void showWidget(const QString &name);
    void showAllWidget();

    void removeWidget(const QString &name);
    void removeWidget(Position pos);

    QString getCentralWidgetName();
    QStringList getCurrentDockName(Position pos);

    void setDockHeaderName(const QString &dockName, const QString &headerName);
    void setDockHeaderList(const QString &dockName, const QList<QAction *> &actions);
    void deleteDockHeader(const QString &name);
    void addToolBtnToDockHeader(const QString &dockName, DToolButton *btn);
    void setDockWidgetFeatures(const QString &name, QDockWidget::DockWidgetFeatures feature);

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
    Position positionOfDock(const QString &dockName);

signals:
    void dockHidden(const QString &dockName);

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
