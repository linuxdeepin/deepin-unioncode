// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <DTitlebar>
#include <DStatusBar>
#include <DToolBar>

#include <QHBoxLayout>
using dpfservice::Position;

DWIDGET_USE_NAMESPACE

class MainWindowPrivate
{
    QMap<QString, QDockWidget *> dockList;

    DWidget *topToolbar { nullptr };
    QHBoxLayout *leftHlayout { nullptr };
    QHBoxLayout *middleHlayout { nullptr };
    QHBoxLayout *rightHlayout { nullptr };

    QMap<QString, DWidget *> topToolList;
    QMap<QString, DWidget *> centralWidgets;

    QString centralWidgetName;
    DMenu *menu { nullptr };

    friend class MainWindow;
};

Qt::DockWidgetArea MainWindow::positionTodockArea(Position pos)
{
    if (pos == Position::FullWindow || pos == Position::Central) {
        qWarning() << "only converting values of Left、Right、Top、Bottom";
        return Qt::NoDockWidgetArea;
    }

    switch (pos) {
    case Position::Left:
        return Qt::LeftDockWidgetArea;
    case Position::Right:
        return Qt::RightDockWidgetArea;
    case Position::Top:
        return Qt::TopDockWidgetArea;
    case Position::Bottom:
        return Qt::BottomDockWidgetArea;
    default:
        return Qt::LeftDockWidgetArea;
    }
}

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent), d(new MainWindowPrivate)
{
    titlebar()->setTitle("Deepin Union Code");
    titlebar()->setIcon(QIcon::fromTheme("ide"));

    setWindowIcon(QIcon::fromTheme("ide"));
    setAttribute(Qt::WA_DeleteOnClose);

    addTopToolBar();
    setContextMenuPolicy(Qt::NoContextMenu);  //donot show left toolbar`s contextmenu
    //setStyleSheet("QMainWindow::separator { width: 2px; margin: 0px; padding: 0px; }");

    setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
    setCorner(Qt::Corner::TopLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
}

MainWindow::~MainWindow()
{
    delete d;
}

DDockWidget *MainWindow::createDockWidget(DWidget *widget)
{
    DDockWidget *dock = new DDockWidget(this);
    dock->setFeatures(QDockWidget::AllDockWidgetFeatures);

    if (dock->titleBarWidget())
        delete dock->titleBarWidget();

    auto header = new DockHeader(this);
    header->setContentsMargins(10, 8, 8, 8);
    dock->setTitleBarWidget(header);

    dock->setWidget(widget);

    return dock;
}

void MainWindow::setDockHeadername(const QString &dockName, const QString &headerName)
{
    if (!d->dockList.contains(dockName))
        return;

    auto dock = d->dockList[dockName];
    auto titleBar = qobject_cast<DockHeader *>(dock->titleBarWidget());

    if (titleBar)
        titleBar->setHeaderName(headerName);
}

void MainWindow::deleteDockHeader(const QString &name)
{
    if (!d->dockList.contains(name))
        return;

    auto dock = d->dockList[name];
    auto titleBar = dock->titleBarWidget();

    if (titleBar)
        delete titleBar;

    dock->setTitleBarWidget(new QWidget());
}

void MainWindow::addToolBtnToDockHeader(const QString &dockName, DToolButton *btn)
{
    if (!d->dockList.contains(dockName))
        return;

    auto dock = d->dockList[dockName];
    auto titleBar = qobject_cast<DockHeader *>(dock->titleBarWidget());

    if (titleBar)
        titleBar->addToolButton(btn);
}

void MainWindow::addWidget(const QString &name, QWidget *widget, Position pos)
{
    if (d->dockList.contains(name)) {
        qWarning() << "dockWidget-" << name << "is already exist";
        return;
    }

    if (pos == Position::FullWindow)
        hideAllWidget();

    if (pos == Position::Central || pos == Position::FullWindow) {
        if (!d->centralWidgetName.isEmpty() && centralWidget())
            hideWidget(d->centralWidgetName);

        setCentralWidget(widget);
        d->centralWidgetName = name;
        d->centralWidgets.insert(name, widget);
        return;
    }

    auto area = positionTodockArea(pos);
    auto dock = createDockWidget(widget);

    addDockWidget(area, dock);

    //initial dock size , modify it as other dock which on same position
    resizeDock(dock);
    d->dockList.insert(name, dock);

    //add close btn to dock`s header
    initDockHeader(dock, pos);
}

void MainWindow::addWidget(const QString &name, QWidget *widget, dpfservice::Position pos, Qt::Orientation orientation)
{
    if (d->dockList.contains(name)) {
        qWarning() << "dockWidget-" << name << "is already exist";
        return;
    }

    if (pos == Position::Central || pos == Position::FullWindow) {
        addWidget(name, widget, pos);
        return;
    }

    auto area = positionTodockArea(pos);
    auto dock = createDockWidget(widget);

    addDockWidget(area, dock, orientation);

    //initial dock size , modify it as other dock which on same position
    resizeDock(dock);
    d->dockList.insert(name, dock);

    //add close btn to dock`s header
    initDockHeader(dock, pos);
}

void MainWindow::initDockHeader(DDockWidget *dock, dpfservice::Position pos)
{
    if (pos != Position::Left)
        return;

    auto closeBtn = new DToolButton(dock);
    closeBtn->setCheckable(true);
    closeBtn->setIcon(QIcon::fromTheme("hide_dock"));
    closeBtn->setToolTip(tr("Hide Dock Widget"));
    closeBtn->setCheckable(false);

    addToolBtnToDockHeader(d->dockList.key(dock), closeBtn);

    connect(closeBtn, &DToolButton::clicked, dock, [=](){
        if (dock->isVisible()) {
            dock->hide();
        } else {
            dock->show();
            resizeDock(d->dockList.key(dock), QSize(300, 300));
        }
    });
}

void MainWindow::resizeDock(QDockWidget *dock)
{
    auto area = dockWidgetArea(dock);

    QSize size(300, 300);
    foreach (auto name, d->dockList.keys()) {
        if (dockWidgetArea(d->dockList[name]) == area) {
            size = d->dockList[name]->size();
            break;
        }
    }

    //(100, 30) means dockWidget havn`t init
    if(size == QSize(100, 30))
        size = QSize(300, 300);

    if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
        resizeDocks({ dock }, { size.width() }, Qt::Horizontal);
    else
        resizeDocks({ dock }, { size.height() }, Qt::Vertical);
}

void MainWindow::resizeDock(const QString &dockName, QSize size)
{
    if (!d->dockList.contains(dockName))
        return;

    auto dock = d->dockList[dockName];
    auto area = dockWidgetArea(dock);
    if (area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea)
        resizeDocks({ dock }, { size.width() }, Qt::Horizontal);
    else
        resizeDocks({ dock }, { size.height() }, Qt::Vertical);
}

void MainWindow::replaceWidget(const QString &name, QWidget *widget, Position pos)
{
    if (d->dockList.contains(name)) {
        qWarning() << "no dockWidget named: " << name;
        return;
    }

    if (pos != Position::FullWindow)
        hideWidget(pos);
    addWidget(name, widget, pos);
}

void MainWindow::removeWidget(Position pos)
{
    if (pos == Position::FullWindow)
        removeAllDockWidget();

    if (pos == Position::Central || pos == Position::FullWindow) {
        auto widget = takeCentralWidget();
        delete widget;
        return;
    }

    auto area = positionTodockArea(pos);
    foreach (auto name, d->dockList.keys()) {
        if (dockWidgetArea(d->dockList[name]) == area) {
            removeDockWidget(d->dockList[name]);
            d->dockList.remove(name);
        }
    }
}

QString MainWindow::getCurrentDockName(dpfservice::Position pos)
{
    if (pos == Position::Central || pos == Position::FullWindow)
        return d->centralWidgetName;

    auto area = positionTodockArea(pos);
    for (auto dock : d->dockList.values()) {
        if (dockWidgetArea(dock) == area && dock->isVisible() == true)
            return d->dockList.key(dock);
    }

    return QString();
}

void MainWindow::setDockWidgetFeatures(const QString &name,QDockWidget::DockWidgetFeatures feature)
{
    if(!d->dockList.contains(name)) {
        qWarning() << name << " is not a dockWidget.";
        return;
    }
    d->dockList[name]->setFeatures(feature);
}

void MainWindow::removeWidget(const QString &name)
{
    if (name == d->centralWidgetName) {
        auto widget = takeCentralWidget();
        delete widget;
        return;
    }
    if (!d->dockList.contains(name)) {
        qWarning() << "no dockWidget named: " << name;
        return;
    }

    removeDockWidget(d->dockList[name]);
    d->dockList.remove(name);
}

void MainWindow::hideWidget(const QString &name)
{
    if (d->centralWidgetName == name && centralWidget()) {
        auto central = takeCentralWidget();
        central->setParent(this);
        central->hide();
        return;
    }

    if (!d->dockList.contains(name)) {
        qWarning() << "no dockWidget named: " << name;
        return;
    }

    d->dockList[name]->setVisible(false);
}

void MainWindow::hideWidget(Position pos)
{
    if (pos == Position::FullWindow) {
        hideAllWidget();
        return;
    }

    if (pos == Position::Central) {
        if(centralWidget())
            hideWidget(d->centralWidgetName);
        return;
    }

    auto area = positionTodockArea(pos);
    foreach (auto name, d->dockList.keys()) {
        if (dockWidgetArea(d->dockList[name]) == area) {
            d->dockList[name]->hide();
        }
    }
}

void MainWindow::hideAllWidget()
{
    foreach (auto dock, d->dockList.values())
        dock->hide();

    if (!d->centralWidgetName.isEmpty() && centralWidget()) {
        hideWidget(d->centralWidgetName);
    }
}

void MainWindow::showWidget(const QString &name)
{
    if (d->centralWidgets.contains(name) && !d->centralWidgetName.isEmpty()) {
        //Prevent the dock widget being stretched when switching central widget
        QList<QDockWidget *> restoreDock;
        foreach (auto dock, d->dockList) {
            if(dock->isVisible()) {
                dock->setVisible(false);
                restoreDock.append(dock);
            }
        }

        if (centralWidget())
            hideWidget(d->centralWidgetName);

        auto central = d->centralWidgets[name];
        d->centralWidgetName = name;
        central->show();
        setCentralWidget(central);

        foreach (auto dock, restoreDock)
            dock->setVisible(true);

        return;
    }

    if (!d->dockList.contains(name)) {
        qWarning() << "no dockWidget named: " << name;
        return;
    }

    d->dockList[name]->setVisible(true);
}

void MainWindow::showWidget(Position pos)
{
    if (pos == Position::FullWindow) {
        hideAllWidget();
    }

    if ((pos == Position::Central || pos == Position::FullWindow) && centralWidget()) {
        showWidget(d->centralWidgetName);
        return;
    }

    auto area = positionTodockArea(pos);
    foreach (auto name, d->dockList.keys()) {
        if (dockWidgetArea(d->dockList[name]) == area) {
            d->dockList[name]->show();
        }
    }
}

void MainWindow::showAllWidget()
{
    foreach (auto dock, d->dockList.values())
        dock->show();
    showWidget(d->centralWidgetName);
}

void MainWindow::removeAllDockWidget()
{
    for (int i = 0; i < d->dockList.count(); i++)
        removeDockWidget(d->dockList.values().at(i));

    d->dockList.clear();
}

void MainWindow::splitWidgetOrientation(const QString &first, const QString &second, Qt::Orientation orientation)
{
    if (!d->dockList.contains(first) || !d->dockList.contains(second))
        return;

    splitDockWidget(d->dockList[first], d->dockList[second], orientation);
}

void MainWindow::setToolbar(Qt::ToolBarArea area, QWidget *widget)
{
    DToolBar *tb = new DToolBar(this);
    tb->setContentsMargins(0, 0, 0, 0);
    tb->layout()->setMargin(0);
    tb->setMovable(false);
    tb->setFloatable(false);

    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tb->addWidget(widget);
    addToolBar(area, tb);
}

void MainWindow::addTopToolBar()
{
    if (d->topToolbar)
        return;

    d->leftHlayout = new QHBoxLayout;
    d->leftHlayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    d->middleHlayout = new QHBoxLayout;
    d->middleHlayout->setAlignment(Qt::AlignCenter);
    d->rightHlayout = new QHBoxLayout;
    d->rightHlayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    d->topToolbar = new DWidget(this);
    d->topToolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *hl = new QHBoxLayout(d->topToolbar);
    hl->setContentsMargins(10, 0, 20, 0);
    hl->addLayout(d->leftHlayout, 1);
    hl->addLayout(d->middleHlayout, 1);
    hl->addLayout(d->rightHlayout, 1);


    QHBoxLayout *titleBarLayout = static_cast<QHBoxLayout *>(titlebar()->layout());
    titleBarLayout->insertWidget(1, d->topToolbar);
}


void MainWindow::setLeftTopToolWidget(DWidget *widget)
{
    if(!d->leftHlayout)
        return;

    d->leftHlayout->addWidget(widget);
}

void MainWindow::setMiddleTopToolWidget(DWidget *widget)
{
    if(!d->middleHlayout)
        return;

    d->middleHlayout->addWidget(widget);
}

void MainWindow::setRightTopToolWidget(DWidget *widget)
{
    if(!d->rightHlayout)
        return;

    d->rightHlayout->addWidget(widget);
}

void MainWindow::showTopToolBar()
{
    d->topToolbar->show();

    titlebar()->setTitle(QString(""));
}

void MainWindow::hideTopTollBar()
{
    d->topToolbar->hide();

    titlebar()->setTitle(QString("Deepin Union Code"));
}
