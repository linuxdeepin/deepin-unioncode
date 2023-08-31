// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

enum Columns { Name, LoadStatus };

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

        case LoadStatus:
            if (role == Qt::CheckStateRole)
                return plugin->isEnabledBySettings() ? Qt::Checked : Qt::Unchecked;
            if (role == Qt::ToolTipRole)
                return PluginView::tr("Load the plugin at startup");
            break;
        }

        return QVariant();
    }

    void setData(int column, int role, const QVariant &data) override
    {
        if (column == LoadStatus && role == Qt::CheckStateRole)
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

        if (column == LoadStatus) {
            if (role == Qt::ToolTipRole)
                return PluginView::tr("Load the plugin at startup");
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
    categoryWidegt->header()->setSortIndicator(0, Qt::AscendingOrder);
    categoryWidegt->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    categoryWidegt->setSelectionMode(QAbstractItemView::SingleSelection);
    categoryWidegt->setSelectionBehavior(QAbstractItemView::SelectRows);
    categoryWidegt->setHeaderLabels({QObject::tr("Name"), QObject::tr("Load Status")});

    auto *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(2, 2, 2, 2);
    gridLayout->addWidget(categoryWidegt);

    QObject::connect(LifeCycle::getPluginManagerInstance(), &PluginManager::pluginsChanged, this,
                     &PluginView::updatePluginsWidegt);

    QObject::connect(categoryWidegt, &QTreeWidget::clicked, [this](){
        if (dynamic_cast<PluginItem *>(categoryWidegt->currentItem()))
            emit currentPluginActived();
    });

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

    categoryWidegt->setCurrentItem(categoryWidegt->topLevelItem(0)->child(0));
}

void PluginView::setPluginEnabled(PluginMetaObjectPointer plugin, bool enable)
{
    plugin->setEnabledBySettings(enable);
    emit pluginSettingChanged(plugin);
}

DPF_END_NAMESPACE
