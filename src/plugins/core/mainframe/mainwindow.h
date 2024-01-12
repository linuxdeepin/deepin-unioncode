// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <DMenu>

#include <QObject>
#include <QWidgetList>
#include <QMap>
#include <QString>

DWIDGET_USE_NAMESPACE

class MainWindowPrivate;
class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    enum Position {
        FullWindow = 0x0,
        Left = 0x1,
        Right = 0x2,
        Top = 0x3,
        Bottom = 0x4,
        Central = 0x5
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //dockWidget
    void addWidget(const QString &name, QWidget *widget, Position pos = FullWindow);

    void replaceWidget(const QString &name, QWidget *widget, Position pos = FullWindow);

    void hideWidget(const QString &name);
    void showWidget(const QString &name);
    void removeWidget(const QString &name);
    void removeWidget(Position pos);

    //只能对四周的widget生效，centralWidget不生效
    void splitWidgetOrientation(const QString &first, const QString &second, Qt::Orientation orientation);

    //toolbar
    void setToolbar(Qt::ToolBarArea area, QWidget *widget);

    //topToolBar - titleBar
    void clearTopTollBar();
    void addTopToolItem(const QString &name, QAction *action);
    void addTopToolBarSpacing(int spacing);

    static Qt::DockWidgetArea positionTodockArea(Position pos);

    //menu
    DMenu *getMenu();

private:
    MainWindowPrivate *d;

    DIconButton *createIconButton(QAction *action);
    void addTopToolBar();
    void removeAllDockWidget();
};
Q_DECLARE_METATYPE(MainWindow::Position)

#endif   // MAINWINDOW_H
