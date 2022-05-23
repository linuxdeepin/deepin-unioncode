/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "filebrowser.h"
#include "mainframe/treeviewkeeper.h"
#include "base/abstractaction.h"
#include "base/abstractcentral.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"

#include <QLabel>
#include <QSplitter>
#include <QWidget>
#include <QTextEdit>
#include <QTreeView>
#include <QGridLayout>
#include <QTabWidget>

using namespace dpfservice;

namespace  {
const int treeWidgtMinWidth = 70;
const int treeWidgetMinHeight = 400;
const QString FileBrowser_TEXT = QWidget::tr("File Browser");
}

void FileBrowser::initialize()
{

}

bool FileBrowser::start()
{
    auto &ctx = dpfInstance.serviceContext();
    WindowService *windowService = ctx.service<WindowService>(WindowService::name());
    if (windowService && windowService->addWidgetWorkspace) {
        windowService->addWidgetWorkspace(FileBrowser_TEXT, createTreeWidget());
    }
    return true;
}

dpf::Plugin::ShutdownFlag FileBrowser::stop()
{
    return Sync;
}

AbstractWidget *FileBrowser::createTreeWidget()
{
    QWidget *treeWidget = new QWidget();
    treeWidget->setMinimumSize({treeWidgtMinWidth, treeWidgetMinHeight});

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);

    auto treeView = TreeViewKeeper::instance()->treeView();
    treeView->setMinimumSize({treeWidgtMinWidth, treeWidgetMinHeight});

    auto folderLabel = new QLabel();
    QObject::connect(treeView, &TreeView::rootPathChanged,
                     folderLabel, &QLabel::setText,
                     Qt::UniqueConnection);

    gridLayout->addWidget(folderLabel);
    gridLayout->addWidget(treeView);
    treeWidget->setLayout(gridLayout);

    return new AbstractWidget(treeWidget);
}


