#include "naveditmainwindow.h"
#include "autohidedockwidget.h"
#include "base/abstractwidget.h"
#include "base/abstractcentral.h"
#include "base/abstractconsole.h"
#include "common/common.h"

#include <QDebug>
#include <QDockWidget>
#include <QEvent>
#include <QWidget>

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
    qDockWidgetContext = new AutoHideDockWidget(QDockWidget::tr("Context"), this);
    qDockWidgetContext->setFeatures(QDockWidget::DockWidgetMovable);
    qTabWidgetContext = new QTabWidget(qDockWidgetContext);
    qDockWidgetContext->setWidget(qTabWidgetContext);
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, qDockWidgetContext);
}

NavEditMainWindow::~NavEditMainWindow()
{
    qTabWidgetContext->removeTab(findIndex(qTabWidgetContext, CONSOLE_TAB_TEXT));
    qInfo() << __FUNCTION__;
}

QStringList NavEditMainWindow::contextWidgetTitles() const
{
    QStringList result;
    for (int index = 0; index < qTabWidgetContext->count(); index ++) {
        result<< qTabWidgetContext->tabText(index);
    }
    return result;
}

void NavEditMainWindow::setConsole(AbstractConsole *console)
{
    QWidget *qConsoleWidget = static_cast<QWidget*>(console->qWidget());
    if (!qConsoleWidget || !qTabWidgetContext) {
        return;
    }

    int consoleIndex = findIndex(qTabWidgetContext, CONSOLE_TAB_TEXT);
    if (consoleIndex >= 0) {
        qTabWidgetContext->removeTab(consoleIndex);
        qTabWidgetContext->insertTab(consoleIndex, qConsoleWidget, CONSOLE_TAB_TEXT);
        return;
    }
    qConsoleWidget->setParent(qTabWidgetContext);
    qTabWidgetContext->insertTab(0, qConsoleWidget, CONSOLE_TAB_TEXT);
}

void NavEditMainWindow::addWidgetWorkspace(const QString &title, AbstractWidget *treeWidget)
{
    if (!qDockWidgetWorkspace) {
        qTabWidgetWorkspace = new QTabWidget();
        qTabWidgetWorkspace->setTabPosition(QTabWidget::West);
        qDockWidgetWorkspace = new AutoHideDockWidget(QDockWidget::tr("Workspace"), this);
        qDockWidgetWorkspace->setFeatures(QDockWidget::DockWidgetMovable);
        qDockWidgetWorkspace->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, qDockWidgetWorkspace);
        qDockWidgetWorkspace->setWidget(qTabWidgetWorkspace);
    }

    if (qTabWidgetWorkspace) {
        auto qTreeWidget = (QWidget*)treeWidget->qWidget();
        qTabWidgetWorkspace->addTab(qTreeWidget, title);
    }
}

void NavEditMainWindow::setWidgetEdit(AbstractCentral *editWidget)
{
    if (centralWidget()) {
        delete qWidgetEdit;
        qWidgetEdit = nullptr;
    }
    qWidgetEdit = (QWidget*)editWidget->qWidget();
    setCentralWidget(qWidgetEdit);
}

void NavEditMainWindow::setWidgetWatch(AbstractWidget *watchWidget)
{
    if (!qDockWidgetWatch) {
        qDockWidgetWatch = new AutoHideDockWidget(QDockWidget::tr("Watcher"), this);
        qDockWidgetWatch->setFeatures(QDockWidget::AllDockWidgetFeatures);
        qDockWidgetWatch->hide();
        addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, qDockWidgetWatch);
    }
    if (qDockWidgetWatch) {
        if (qWidgetWatch) {
            delete qWidgetWatch;
            qWidgetWatch = nullptr;
        }
        qWidgetWatch = static_cast<QWidget*>(watchWidget->qWidget());
        qWidgetWatch->setParent(qDockWidgetWatch);
        qDockWidgetWatch->setWidget(qWidgetWatch);
        qDockWidgetWatch->hide();
    }
}

void NavEditMainWindow::addWidgetContext(const QString &title, AbstractWidget *contextWidget)
{
    QWidget *qWidget = static_cast<QWidget*>(contextWidget->qWidget());
    if (!qWidget || !qTabWidgetContext) {
        return;
    }
    qTabWidgetContext->addTab(qWidget, title);
}

bool NavEditMainWindow::switchWidgetContext(const QString &title)
{
    for (int i = 0; i < qTabWidgetContext->count(); i++){
        if (qTabWidgetContext->tabText(i) == title) {
            qTabWidgetContext->setCurrentIndex(i);
            return true;
        }
    }
    return false;
}
