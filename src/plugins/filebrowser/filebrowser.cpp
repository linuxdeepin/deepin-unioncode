// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filebrowser.h"
#include "mainframe/treeviewkeeper.h"
#include "base/abstractaction.h"
#include "base/abstractwidget.h"
#include "services/window/windowservice.h"

#include <DFrame>
#include <DStyle>
#include <DWidget>
#include <DLabel>
#include <DTreeView>
#include <DTabWidget>
#include <DTextEdit>
#include <DSplitter>

#include <QGridLayout>

DWIDGET_USE_NAMESPACE

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
        windowService->addWidgetWorkspace(FileBrowser_TEXT, createTreeWidget(), "edit-file");
    }
    return true;
}

dpf::Plugin::ShutdownFlag FileBrowser::stop()
{
    return Sync;
}

AbstractWidget *FileBrowser::createTreeWidget()
{
    DFrame *treeWidget = new DFrame();
    treeWidget->setLineWidth(0);
    DStyle::setFrameRadius(treeWidget, 0);

    auto treeView = TreeViewKeeper::instance()->treeView();
    auto folderLabel = new DLabel();
    folderLabel->setMargin(3);
    folderLabel->setElideMode(Qt::TextElideMode::ElideMiddle);
    QObject::connect(treeView, &TreeView::rootPathChanged,
                     folderLabel, &DLabel::setText,
                     Qt::UniqueConnection);

    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addWidget(folderLabel);
    vLayout->addWidget(new DHorizontalLine());
    vLayout->addWidget(treeView);
    vLayout->addSpacing(3);
    treeWidget->setLayout(vLayout);

    return new AbstractWidget(treeWidget);
}


