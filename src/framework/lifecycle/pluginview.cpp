/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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


#include "lifecycle.h"
#include "pluginview.h"
#include "pluginmetaobject.h"

#include <QTreeWidget>
#include <QAbstractItemModel>
#include <QHeaderView>
#include <QGridLayout>
#include <QItemSelectionModel>
#include <QTreeWidgetItem>
#include <QStandardItem>

DPF_BEGIN_NAMESPACE

enum Columns { Name, Load, Version, Vendor };

class PluginItem : public QTreeWidgetItem
{
public:
    PluginItem(PluginMetaObjectPointer plugin, PluginView *view)
        : name(""), plugin(plugin), view(view)
    {}

    QVariant data(int column, int role) const override
    {
        switch (column) {
        case Name:
            if (role == Qt::DisplayRole)
                return plugin->name();
            if (role == Qt::ToolTipRole) {
                QString toolTip = PluginView::tr("Path: %1");
                return toolTip.arg(QDir::toNativeSeparators(plugin->fileName()));
            }
            break;

        case Load:
            if (role == Qt::CheckStateRole)
                return plugin->isEnabledBySettings() ? Qt::Checked : Qt::Unchecked;
            if (role == Qt::ToolTipRole)
                return PluginView::tr("Load on startup");
            break;

        case Version:
            if (role == Qt::DisplayRole)
                return QString::fromLatin1("%1 (%2)").arg(plugin->version(), plugin->compatVersion());
            break;

        case Vendor:
            if (role == Qt::DisplayRole)
                return plugin->vendor();
            break;
        }

        return QVariant();
    }

    void setData(int column, int role, const QVariant &data) override
    {
        if (column == Load && role == Qt::CheckStateRole)
            view->setPluginEnabled(plugin, data.toBool());
    }

public:
    QString name;
    PluginMetaObjectPointer plugin;
    PluginView *view = nullptr;
};

class CollectionItem : public QTreeWidgetItem
{
public:
    CollectionItem(const QString &name, QList<PluginMetaObjectPointer> plugins, PluginView *view)
        : name(name), plugins(plugins), view(view)
    {
        for (PluginMetaObjectPointer plugin : plugins)
            this->addChild(new PluginItem(plugin, view));
    }

    QVariant data(int column, int role) const override
    {
        if (column == Name) {
            if (role == Qt::DisplayRole)
                return name;
        }

        if (column == Load) {
            if (role == Qt::ToolTipRole)
                return PluginView::tr("Load on Startup");
        }

        return QVariant();
    }

public:
    QString name;
    QList<PluginMetaObjectPointer> plugins;
    PluginView *view = nullptr;
};

PluginView::PluginView(QWidget *parent)
    : QWidget(parent)
{
    categoryWidegt = new QTreeWidget(this);
    categoryWidegt->setIndentation(20);
    categoryWidegt->setUniformRowHeights(true);
    categoryWidegt->setSortingEnabled(true);
    categoryWidegt->setColumnWidth(2, 40);
    categoryWidegt->header()->setDefaultSectionSize(120);
    categoryWidegt->header()->setMinimumSectionSize(40);
    categoryWidegt->header()->setSortIndicator(0, Qt::AscendingOrder);
    categoryWidegt->setSelectionMode(QAbstractItemView::SingleSelection);
    categoryWidegt->setSelectionBehavior(QAbstractItemView::SelectRows);
    categoryWidegt->setHeaderLabels({ tr("Name"), tr("Load"), tr("Version"), tr("Vendor") });

    auto *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(2, 2, 2, 2);
    gridLayout->addWidget(categoryWidegt);

    QHeaderView *header = categoryWidegt->header();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    QObject::connect(LifeCycle::getPluginManagerInstance(), &PluginManager::pluginsChanged, this,
                     &PluginView::updatePluginsWidegt);

    QObject::connect(categoryWidegt, &QTreeWidget::activated,
                     [this](){ emit currentPluginActived(); });

    QObject::connect(categoryWidegt->selectionModel(), &QItemSelectionModel::currentChanged,
                     [this](){ emit currentPluginChanged(); });

    updatePluginsWidegt();
}

PluginMetaObjectPointer PluginView::currentPlugin() const
{
    QTreeWidgetItem *sourceItem = categoryWidegt->currentItem();
    if (!sourceItem)
        return nullptr;

    CollectionItem *collectItem = static_cast<CollectionItem*>(sourceItem);
    QString name = collectItem->name;
    if (name.isEmpty()) {
        PluginItem *plugitem = static_cast<PluginItem*>(sourceItem);
        return plugitem ? plugitem->plugin : nullptr;
    }

    return nullptr;
}

void PluginView::updatePluginsWidegt()
{
    QList<CollectionItem *> collections;
    const QHash<QString, QQueue<PluginMetaObjectPointer>> pluginCollections = LifeCycle::getPluginManagerInstance()->pluginCollections();
    const auto end = pluginCollections.cend();
    for (auto it = pluginCollections.begin(); it != end; ++it) {
        const QString name = it.key();
        if ("Core Plugins" != name)
            collections.append(new CollectionItem(name, it.value(), this));
    }

    for (CollectionItem *collection : collections) {
        categoryWidegt->invisibleRootItem()->addChild(collection);
    }

    categoryWidegt->expandAll();
}

void PluginView::setPluginEnabled(PluginMetaObjectPointer plugin, bool enable)
{
    plugin->setEnabledBySettings(enable);
    emit pluginSettingChanged(plugin);
}

DPF_END_NAMESPACE
