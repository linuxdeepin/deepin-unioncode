/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "naveditframe.h"
#include "base/abstractwidget.h"
#include "base/abstractconsole.h"
#include "textedittabwidget/textedittabwidget.h"

#include <QLabel>
#include <QSplitter>
#include <QWidget>
#include <QTextEdit>
#include <QTreeView>
#include <QGridLayout>
#include <QTabWidget>
#include <QDebug>
#include <QEvent>

const int treeWidgtMinWidth = 100;
const int treeWidgetMinHeight = 400;

const int codeWidgetMinWidth = 400;
const int codeWidgetMinHeight = 200;

const int contextWidgetMinWidth = 400;
const int contextWidgetMinHeight = 240;

const int watchWidgetMinWidth = 200;
const int watchWidgetMinHeight = 200;

#define TREEWIDGET_INDEX 0

const QString CONSOLE_TAB_TEXT = NavEditFrame::tr("Console");

int findIndex(QTabWidget* tabWidget, const QString &text)
{
    for (int index = 0; index < tabWidget->count(); index ++) {
        if (tabWidget->tabText(index) == text) {
            return index;
        }
    }

    return -1;
}

NavEditFrame::NavEditFrame(QWidget *parent)
    :QSplitter(parent)
{
    //发布edit导航栏界面布局
    setOrientation(Qt::Horizontal);
    setSizes({treeWidgtMinWidth, treeWidgetMinHeight});
    //右侧纵向分割
    verSplitter = new QSplitter(Qt::Vertical, this);
    horSplitter = new QSplitter(Qt::Horizontal, verSplitter);
    createEditorWidget(horSplitter);
    createContextWidget(verSplitter);
    // setStyleSheet("border:1px solid rgb(255,255,255);");
}

void NavEditFrame::createEditorWidget(QSplitter *splitter)
{
    qInfo() << __FUNCTION__;
    QWidget *codeWidget = new QWidget();
    codeWidget->setMinimumSize({codeWidgetMinWidth, codeWidgetMinHeight});
    splitter->addWidget(codeWidget);
    splitter->setChildrenCollapsible(false);

    NavEditFrame::editorLayout = new QGridLayout();
    editorLayout->setSpacing(0);
    editorLayout->setMargin(0);
    codeWidget->setLayout(editorLayout);

    TextEditTabWidget* edit = new TextEditTabWidget();
    editorLayout->addWidget(edit);
}

void NavEditFrame::createContextWidget(QSplitter *splitter)
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

bool NavEditFrame::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == NavEditFrame::watchWidget) {
        if (e->type() == QEvent::Show) {
            int miniWidth = qMax(NavEditFrame::watchWidget->minimumWidth(), watchWidgetMinWidth);
            NavEditFrame::watchWidget->setMinimumWidth(miniWidth);
            NavEditFrame::watchWidget->setMinimumHeight(watchWidgetMinHeight);
            if (NavEditFrame::horSplitter) {
                NavEditFrame::horSplitter->addWidget(NavEditFrame::watchWidget);
                NavEditFrame::horSplitter->setStretchFactor(0, 8);
                NavEditFrame::horSplitter->setStretchFactor(1, 2);
                return true;
            }
        }
        if (e->type() == QEvent::Hide) {
            int watchWidgetIndex = NavEditFrame::horSplitter->indexOf(NavEditFrame::watchWidget);
            if (NavEditFrame::horSplitter && watchWidgetIndex != -1) {
                NavEditFrame::horSplitter->replaceWidget(watchWidgetIndex, nullptr);
                return true;
            }
        }
    }
    return false;
}

void NavEditFrame::setTreeWidget(AbstractWidget *treeWidget)
{
    qInfo() << __FUNCTION__;
    QWidget *qWidget = static_cast<QWidget*>(treeWidget->qWidget());
    if (!qWidget)
        return;

    if (NavEditFrame::treeWidget) {
        delete NavEditFrame::treeWidget;
        NavEditFrame::treeWidget = nullptr;
    }
    NavEditFrame::treeWidget = qWidget;
    insertWidget(TREEWIDGET_INDEX, qWidget);
    setSizes({treeWidgtMinWidth,treeWidgetMinHeight});
    setChildrenCollapsible(false);
}

void NavEditFrame::setConsole(AbstractConsole *console)
{
    qInfo() << __FUNCTION__;
    QWidget *qWidget = static_cast<QWidget*>(console->qWidget());
    if (!qWidget || !tabWidget) {
        return;
    }

    int consoleIndex = findIndex(tabWidget, CONSOLE_TAB_TEXT);
    if (consoleIndex >= 0) {
        tabWidget->removeTab(consoleIndex);
        tabWidget->insertTab(consoleIndex, qWidget, CONSOLE_TAB_TEXT);
        return;
    }
    qWidget->setParent(this);
    tabWidget->insertTab(0, qWidget, CONSOLE_TAB_TEXT);
}

void NavEditFrame::setWatchWidget(AbstractWidget *watchWidget)
{
    qInfo() << __FUNCTION__;
    QWidget *qWidget = static_cast<QWidget*>(watchWidget->qWidget());
    if (!qWidget || !NavEditFrame::horSplitter) {
        return;
    }

    if (NavEditFrame::watchWidget) {
        delete NavEditFrame::watchWidget;
        NavEditFrame::watchWidget = nullptr;
    }

    NavEditFrame::watchWidget = qWidget;
    qWidget->installEventFilter(this);
}

void NavEditFrame::addContextWidget(const QString &title, AbstractWidget *contextWidget)
{
    qInfo() << __FUNCTION__;
    QWidget *qWidget = static_cast<QWidget*>(contextWidget->qWidget());
    if (!qWidget || !tabWidget) {
        return;
    }

    tabWidget->addTab(qWidget, title);
}
