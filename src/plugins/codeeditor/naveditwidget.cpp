#include "naveditwidget.h"
#include "editwidget.h"
#include "base/abstractwidget.h"

#include <QLabel>
#include <QSplitter>
#include <QWidget>
#include <QTextEdit>
#include <QTreeView>
#include <QGridLayout>
#include <QTabWidget>
#include <QDebug>

const int treeWidgtMinWidth = 100;
const int treeWidgetMinHeight = 400;

const int codeWidgetMinWidth = 400;
const int codeWidgetMinHeight = 200;

const int contextWidgetMinWidth = 400;
const int contextWidgetMinHeight = 240;

const QString CONSOLE_TAB_TEXT = NavEditWidget::tr("Console");

int findIndex(QTabWidget* tabWidget, const QString &text)
{
    for (int index = 0; index < tabWidget->count(); index ++) {
        if (tabWidget->tabText(index) == text) {
            return index;
        }
    }

    return -1;
}

NavEditWidget::NavEditWidget(QWidget *parent)
    :QSplitter(parent)
{
    //发布edit导航栏界面布局
    setOrientation(Qt::Horizontal);
    setSizes({treeWidgtMinWidth, treeWidgetMinHeight});
    //右侧纵向分割
    verSplitter = new QSplitter(Qt::Vertical, this);
    createCodeWidget(verSplitter);
    createContextWidget(verSplitter);
    setStyleSheet("border:1px solid rgb(0,136,221);");
}

void NavEditWidget::createCodeWidget(QSplitter *splitter)
{
    qInfo() << __FUNCTION__;
    QWidget *codeWidget = new QWidget();
    codeWidget->setMinimumSize({codeWidgetMinWidth, codeWidgetMinHeight});
    splitter->addWidget(codeWidget);
    splitter->setChildrenCollapsible(false);

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    codeWidget->setLayout(gridLayout);

    EditWidget* editWidget = new EditWidget();
    gridLayout->addWidget(editWidget);
}

void NavEditWidget::createContextWidget(QSplitter *splitter)
{
    qInfo() << __FUNCTION__;
    QWidget *contextWidget = new QWidget();
    contextWidget->setMinimumSize({contextWidgetMinWidth, contextWidgetMinHeight});
    splitter->addWidget(contextWidget);
    splitter->setChildrenCollapsible(false);

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    contextWidget->setLayout(gridLayout);

    tabWidget = new QTabWidget();
    gridLayout->addWidget(tabWidget);
}

void NavEditWidget::setTreeWidget(AbstractWidget *treeWidget)
{
    qInfo() << __FUNCTION__;
    QWidget *qWidget = static_cast<QWidget*>(treeWidget->qWidegt());
    if (!qWidget)
        return;

    if (count() >= 2) {
        QWidget *srcWidget = widget(0);
        delete srcWidget;
    }

    insertWidget(0, qWidget);
    setSizes({treeWidgtMinWidth,treeWidgetMinHeight});
    setChildrenCollapsible(false);
}

void NavEditWidget::setConsole(AbstractWidget *console)
{
    qInfo() << __FUNCTION__;
    QWidget *qWidget = static_cast<QWidget*>(console->qWidegt());
    if (!qWidget || !tabWidget) {
        return;
    }

    int consoleIndex = findIndex(tabWidget, CONSOLE_TAB_TEXT);
    if (consoleIndex >= 0) {
        tabWidget->removeTab(consoleIndex);
        tabWidget->insertTab(consoleIndex, qWidget, CONSOLE_TAB_TEXT);
        return;
    }

    tabWidget->insertTab(0, qWidget, CONSOLE_TAB_TEXT);
}

void NavEditWidget::addContextWidget(const QString &title, AbstractWidget *contextWidget)
{
    qInfo() << __FUNCTION__;
    QWidget *qWidget = static_cast<QWidget*>(contextWidget->qWidegt());
    if (!qWidget || !tabWidget) {
        return;
    }

    tabWidget->addTab(qWidget, title);
}
