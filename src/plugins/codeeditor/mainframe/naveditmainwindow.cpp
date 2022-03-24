#include "naveditmainwindow.h"
#include "autohidedockwidget.h"
#include "base/abstractwidget.h"
#include "base/abstractcentral.h"
#include "base/abstractconsole.h"
#include <QDebug>
#include <QDockWidget>
#include <QEvent>
#include <QWidget>

const QString CONSOLE_TAB_TEXT = NavEditMainWindow::tr("Console");
static NavEditMainWindow *ins{nullptr};
int findIndex(QTabWidget* tabWidget, const QString &text)
{
    for (int index = 0; index < tabWidget->count(); index ++) {
        if (tabWidget->tabText(index) == text) {
            return index;
        }
    }
    return -1;
}

NavEditMainWindow *NavEditMainWindow::instance()
{
    if (!ins)
        ins = new NavEditMainWindow;
    return ins;
}

NavEditMainWindow::NavEditMainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow (parent, flags)
{
    qTabWidgetDock = new AutoHideDockWidget(QDockWidget::tr("Context"), this);
    qTabWidgetDock->setFeatures(QDockWidget::DockWidgetMovable);
    qTabWidget = new QTabWidget(qTabWidgetDock);
    qTabWidgetDock->setWidget(qTabWidget);
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, qTabWidgetDock);
}

NavEditMainWindow::~NavEditMainWindow()
{
    qTabWidget->removeTab(findIndex(qTabWidget, CONSOLE_TAB_TEXT));
    qInfo() << __FUNCTION__;
}

void NavEditMainWindow::setConsole(AbstractConsole *console)
{
    QWidget *qConsoleWidget = static_cast<QWidget*>(console->qWidget());
    if (!qConsoleWidget || !qTabWidget) {
        return;
    }

    int consoleIndex = findIndex(qTabWidget, CONSOLE_TAB_TEXT);
    if (consoleIndex >= 0) {
        qTabWidget->removeTab(consoleIndex);
        qTabWidget->insertTab(consoleIndex, qConsoleWidget, CONSOLE_TAB_TEXT);
        return;
    }
    qConsoleWidget->setParent(qTabWidget);
    qTabWidget->insertTab(0, qConsoleWidget, CONSOLE_TAB_TEXT);
}

void NavEditMainWindow::setTreeWidget(AbstractWidget *treeWidget)
{
    if (!qTreeWidgetDock) {
        qTreeWidgetDock = new AutoHideDockWidget(QDockWidget::tr("Workspace"), this);
        qTreeWidgetDock->setFeatures(QDockWidget::DockWidgetMovable);
        qTreeWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, qTreeWidgetDock);
    }

    if (qTreeWidgetDock) {
        if (qTreeWidget) {
            delete qTreeWidget;
            qTreeWidget = nullptr;
        }
        qTreeWidget = (QWidget*)treeWidget->qWidget();
        qTreeWidget->setParent(qTreeWidgetDock);
        qTreeWidgetDock->setWidget(qTreeWidget);
    }
}

void NavEditMainWindow::setEditWidget(AbstractCentral *editWidget)
{
    if (centralWidget()) {
        delete qEditWidget;
        qEditWidget = nullptr;
    }
    qEditWidget = (QWidget*)editWidget->qWidget();
    setCentralWidget(qEditWidget);
}

void NavEditMainWindow::setWatchWidget(AbstractWidget *watchWidget)
{
    if (!qWatchWidgetDock) {
        qWatchWidgetDock = new AutoHideDockWidget(QDockWidget::tr("Watcher"), this);
        qWatchWidgetDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
        qWatchWidgetDock->hide();
        addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, qWatchWidgetDock);
    }
    if (qWatchWidgetDock) {
        if (qWatchWidget) {
            delete qWatchWidget;
            qWatchWidget = nullptr;
        }
        qWatchWidget = static_cast<QWidget*>(watchWidget->qWidget());
        qWatchWidget->setParent(qWatchWidgetDock);
        qWatchWidgetDock->setWidget(qWatchWidget);
        qWatchWidgetDock->hide();
    }
}

void NavEditMainWindow::addContextWidget(const QString &title, AbstractWidget *contextWidget)
{
    QWidget *qWidget = static_cast<QWidget*>(contextWidget->qWidget());
    if (!qWidget || !qTabWidget) {
        return;
    }
    qTabWidget->addTab(qWidget, title);
}

bool NavEditMainWindow::switchContextWidget(const QString &title)
{
    for (int i = 0; i < qTabWidget->count(); i++){
        if (qTabWidget->tabText(i) == title) {
            qTabWidget->setCurrentIndex(i);
            return true;
        }
    }
    return false;
}
