// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginsui.h"

#include "framework/lifecycle/lifecycle.h"
#include "framework/lifecycle/pluginmanager.h"
#include "framework/lifecycle/pluginmetaobject.h"
#include "pluginstorewidget.h"
#include "pluginlistview.h"
#include "plugindetailsview.h"

#include <DTitlebar>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopServices>

static bool isRestartRequired = false;

PluginsUi::PluginsUi(QObject *parent)
    : QObject(parent)
    , pluginStoreWidget(new PluginStoreWidget())
    , pluginDetailView(new DetailsView())
{
    auto pluginListView = pluginStoreWidget->getPluginListView();
    QObject::connect(pluginListView, &PluginListView::currentPluginActived,
                     this, &PluginsUi::slotPluginItemSelected);

    QObject::connect(pluginListView, &PluginListView::pluginSettingChanged,
                     this, &PluginsUi::reLaunchRequired);

    // display detail default.
    emit pluginListView->currentPluginActived();
}

PluginsUi::~PluginsUi()
{
    if (pluginDetailView) {
        delete pluginDetailView;
        pluginDetailView = nullptr;
    }
    if (pluginStoreWidget) {
        delete pluginStoreWidget;
        pluginStoreWidget = nullptr;
    }
}

PluginStoreWidget *PluginsUi::getStoreWidget() const
{
    return pluginStoreWidget;
}

DetailsView *PluginsUi::getPluginDetailView() const
{
    return pluginDetailView;
}

void PluginsUi::slotPluginItemSelected()
{
    auto pluginListView = pluginStoreWidget->getPluginListView();
    dpf::PluginMetaObjectPointer plugin = pluginListView->currentPlugin();
    pluginDetailView->update(plugin);
}

void PluginsUi::reLaunchRequired()
{
    isRestartRequired = true;
    saveConfig();
}

void PluginsUi::saveConfig()
{
    dpf::LifeCycle::getPluginManagerInstance()->writeSettings();
}


