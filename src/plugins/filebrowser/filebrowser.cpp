// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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


