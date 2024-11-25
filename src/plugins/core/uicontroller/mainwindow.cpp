// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "common/util/eventdefinitions.h"
#include "common/util/custompaths.h"

#include <DTitlebar>
#include <DStatusBar>
#include <DToolBar>

#include <QDataStream>
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
    titlebar()->setFocusPolicy(Qt::NoFocus);

    setWindowIcon(QIcon::fromTheme("ide"));
    setStyle(new CustomStyle());
    addTopToolBar();
    setContextMenuPolicy(Qt::NoContextMenu);   //donot show left toolbar`s contextmenu

    setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
    setCorner(Qt::Corner::TopLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
    setCorner(Qt::Corner::BottomRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);
}

MainWindow::~MainWindow()
{
    QString initFile = CustomPaths::user(CustomPaths::Configures) + "/mainwindow.ini";
    QFile file(initFile);
    if (file.open(QFile::WriteOnly)) {
        QDataStream outFile(&file);
        outFile << saveGeometry();
    }

    delete d;
}

DDockWidget *MainWindow::createDockWidget(DWidget *widget)
{
    DDockWidget *dock = new DDockWidget(this);
    dock->setFeatures(QDockWidget::AllDockWidgetFeatures);

    if (dock->titleBarWidget())
        delete dock->titleBarWidget();

    auto header = new DockHeader(this);
    dock->setTitleBarWidget(header);

    dock->setWidget(widget);

    return dock;
}

void MainWindow::setDockHeaderName(const QString &dockName, const QString &headerName)
{
    if (!d->dockList.contains(dockName))
        return;

    auto dock = d->dockList[dockName];
    auto titleBar = qobject_cast<DockHeader *>(dock->titleBarWidget());

    if (titleBar)
        titleBar->setHeaderName(headerName);
}

void MainWindow::setDockHeaderList(const QString &dockName, const QList<QAction *> &actions)
{
    if (!d->dockList.contains(dockName) || actions.isEmpty())
        return;

    auto dock = d->dockList[dockName];
    auto titleBar = qobject_cast<DockHeader *>(dock->titleBarWidget());

    if (titleBar)
        titleBar->setHeaderNames(actions);
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

void MainWindow::addWidgetToDockHeader(const QString &dockName, QWidget *widget)
{
    if (!d->dockList.contains(dockName))
        return;

    auto dock = d->dockList[dockName];
    auto titleBar = qobject_cast<DockHeader *>(dock->titleBarWidget());

    if (titleBar)
        titleBar->addWidget(widget);
}

QDockWidget *MainWindow::addWidget(const QString &name, QWidget *widget, Position pos)
{
    if (d->dockList.contains(name)) {
        qWarning() << "dockWidget-" << name << "is already exist";
        return nullptr;
    }

    if (pos == Position::FullWindow)
        hideAllWidget();

    if (pos == Position::Central || pos == Position::FullWindow) {
        if (!d->centralWidgetName.isEmpty() && centralWidget())
            hideWidget(d->centralWidgetName);

        widget->show();
        setCentralWidget(widget);
        d->centralWidgetName = name;
        d->centralWidgets.insert(name, widget);
        return nullptr;
    }

    auto area = positionTodockArea(pos);
    auto dock = createDockWidget(widget);

    addDockWidget(area, dock);

    //initial dock size , modify it as other dock which on same position
    resizeDock(dock);
    d->dockList.insert(name, dock);

    //add close btn to dock`s header
    initDockHeader(dock, pos);
    return dock;
}

QDockWidget *MainWindow::addWidget(const QString &name, QWidget *widget, dpfservice::Position pos, Qt::Orientation orientation)
{
    if (d->dockList.contains(name)) {
        qWarning() << "dockWidget-" << name << "is already exist";
        return nullptr;
    }

    if (pos == Position::Central || pos == Position::FullWindow) {
        addWidget(name, widget, pos);
        return nullptr;
    }

    auto area = positionTodockArea(pos);
    auto dock = createDockWidget(widget);

    addDockWidget(area, dock, orientation);

    //initial dock size , modify it as other dock which on same position
    resizeDock(dock);
    d->dockList.insert(name, dock);

    //add close btn to dock`s header
    initDockHeader(dock, pos);
    return dock;
}

void MainWindow::initDockHeader(DDockWidget *dock, dpfservice::Position pos)
{
    auto closeBtn = new DToolButton(dock);
    closeBtn->setIcon(QIcon::fromTheme("hide_dock"));
    closeBtn->setToolTip(tr("Hide Dock Widget"));
    closeBtn->setCheckable(false);

    addWidgetToDockHeader(d->dockList.key(dock), closeBtn);

    connect(closeBtn, &DToolButton::clicked, dock, [=]() {
        auto dockName = d->dockList.key(dock);
        if (dock->isVisible()) {
            dock->hide();
        }
        emit dockHidden(dockName);
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
    if (size == QSize(100, 30))
        size = QSize(300, 300);

    resizeDocks({ dock }, { size.width() }, Qt::Horizontal);
    resizeDocks({ dock }, { size.height() }, Qt::Vertical);
}

void MainWindow::resizeDock(const QString &dockName, QSize size)
{
    if (!d->dockList.contains(dockName))
        return;

    auto dock = d->dockList[dockName];

    resizeDocks({ dock }, { size.width() }, Qt::Horizontal);
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

QString MainWindow::getCentralWidgetName()
{
    return d->centralWidgetName;
}

QStringList MainWindow::getCurrentDockName(dpfservice::Position pos)
{
    if (pos == Position::Central || pos == Position::FullWindow)
        return { d->centralWidgetName };

    QStringList ret {};
    auto area = positionTodockArea(pos);
    for (auto dock : d->dockList.values()) {
        if (dockWidgetArea(dock) == area && dock->isVisible() == true)
            ret << d->dockList.key(dock);
    }

    return ret;
}

void MainWindow::setDockWidgetFeatures(const QString &name, QDockWidget::DockWidgetFeatures feature)
{
    if (!d->dockList.contains(name)) {
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
    auto dock = d->dockList[name];
    d->dockList.remove(name);
    delete dock;
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
        if (centralWidget())
            hideWidget(d->centralWidgetName);
        return;
    }

    auto area = positionTodockArea(pos);
    foreach (auto name, d->dockList.keys()) {
        if (dockWidgetArea(d->dockList[name]) == area)
            d->dockList[name]->hide();
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
            if (dock->isVisible()) {
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
    if (!d->leftHlayout)
        return;

    d->leftHlayout->addWidget(widget);
}

void MainWindow::setMiddleTopToolWidget(DWidget *widget)
{
    if (!d->middleHlayout)
        return;

    d->middleHlayout->addWidget(widget);
}

void MainWindow::setRightTopToolWidget(DWidget *widget)
{
    if (!d->rightHlayout)
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

Position MainWindow::positionOfDock(const QString &dockName)
{
    if (!d->dockList.contains(dockName)) {
        qWarning() << "no dockWidget named: " << dockName;
        return Position::FullWindow;
    }

    auto area = dockWidgetArea(d->dockList[dockName]);
    switch (area) {
    case Qt::LeftDockWidgetArea:
        return Position::Left;
    case Qt::RightDockWidgetArea:
        return Position::Right;
    case Qt::TopDockWidgetArea:
        return Position::Top;
    case Qt::BottomDockWidgetArea:
        return Position::Bottom;
    default:
        return Position::Left;
    }
}
