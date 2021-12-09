/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#include "codeedit.h"
#include "treeview.h"
#include "treeproxy.h"
#include "editwidget.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "services/window/windowservice.h"

#include <QLabel>
#include <QSplitter>
#include <QWidget>
#include <QTextEdit>
#include <QTreeView>
#include <QGridLayout>
#include <QTabWidget>

using namespace dpfservice;

const int treeWidgtMinWidth = 70;
const int treeWidgetMinHeight = 400;

const int codeWidgetMinWidth = 400;
const int codeWidgetMinHeight = 200;

const int contextWidgetMinWidth = 400;
const int contextWidgetMinHeight = 240;

void CodeEdit::initialize()
{

}

bool CodeEdit::start()
{
    auto &ctx = dpfInstance.serviceContext();
    qInfo() << "import service list" <<  ctx.services();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());

    QSplitter *horSplitter = new QSplitter(Qt::Horizontal);
    createTreeWidget(horSplitter);
    horSplitter->setSizes({treeWidgtMinWidth, treeWidgetMinHeight});
    QSplitter *verSplitter = new QSplitter(Qt::Vertical, horSplitter);
    createCodeWidget(verSplitter);
    createContextWidget(verSplitter);

    horSplitter->setStyleSheet("border:1px solid rgb(127,255,212);");
    windowService->addCentral(QString::fromStdString(NAVACTION_EDIT), new AbstractCentral(horSplitter));
    return true;
}

dpf::Plugin::ShutdownFlag CodeEdit::stop()
{
    return Sync;
}

void CodeEdit::createTreeWidget(QSplitter *splitter)
{
    QWidget *treeWidget = new QWidget();
    treeWidget->setMinimumSize({treeWidgtMinWidth, treeWidgetMinHeight});
    splitter->addWidget(treeWidget);
    splitter->setChildrenCollapsible(false);

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    treeWidget->setLayout(gridLayout);

    TreeView* treeView = new TreeView();
    treeView->setMinimumSize({treeWidgtMinWidth, treeWidgetMinHeight});
    gridLayout->addWidget(treeView);
}

void CodeEdit::createCodeWidget(QSplitter *splitter)
{
    QWidget *codeWidget = new QWidget();
    codeWidget->setMinimumSize({codeWidgetMinWidth, codeWidgetMinHeight});
    splitter->addWidget(codeWidget);
    splitter->setChildrenCollapsible(false);

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    codeWidget->setLayout(gridLayout);

    EditWidget* editWidget = new EditWidget();
    editWidget->append("code test edit");
    gridLayout->addWidget(editWidget);
}

void CodeEdit::createContextWidget(QSplitter *splitter)
{
    QWidget *contextWidget = new QWidget();
    contextWidget->setMinimumSize({contextWidgetMinWidth, contextWidgetMinHeight});
    splitter->addWidget(contextWidget);
    splitter->setChildrenCollapsible(false);

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    contextWidget->setLayout(gridLayout);

    QTabWidget* tabWidget = new QTabWidget();
    gridLayout->addWidget(tabWidget);
}


