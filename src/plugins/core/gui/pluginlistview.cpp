// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "pluginlistview.h"
#include "framework/lifecycle/lifecycle.h"
#include "pluginitemdelegate.h"

#include <QStandardItemModel>
#include <DStandardItem>

DPF_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
PluginListView::PluginListView(QListView *parent)
    : QListView(parent)
    , model(new QStandardItemModel(this))
{
    setModel(model);
    setItemDelegate(new PluginItemDelegate(this));

    QObject::connect(LifeCycle::getPluginManagerInstance(), &PluginManager::pluginsChanged, this,
                     &PluginListView::display);

    QObject::connect(this, &QListView::clicked, [this](){
            emit currentPluginActived();
    });

    display();
}

void PluginListView::display()
{
    const QHash<QString, QQueue<PluginMetaObjectPointer>> &pluginCollections =
            LifeCycle::getPluginManagerInstance()->pluginCollections();

    model->clear();
    QList<QStandardItem*> result;
    for (auto collection : pluginCollections) {
        for (auto metaOBject : collection) {
            QString pluginName = metaOBject->name();
            QString description = metaOBject->description();
            QString vender = metaOBject->vendor();
            QString category = metaOBject->category();
            QString copyright = metaOBject->copyright();
            QStringList license = metaOBject->license();
            QString version = metaOBject->version();

            auto rowItem = new DStandardItem(QIcon::fromTheme("plugins-navigation"), pluginName);
            rowItem->setData(description, PluginListView::Description);
            rowItem->setData(vender, PluginListView::Vender);
            rowItem->setData(category, PluginListView::Category);

            QString toolTip;
            QTextStream textStrem(&toolTip);
            textStrem << "<html><body><b>" << pluginName << "  </b><em>" << version << "</em></font><hr>"
                << description << "<hr>"
                << "<b>" << tr("Category") << "</b>" << " : " << category << "<br>"
                << "<b>" << tr("License") << "</b>" << " : " << license.join("") << "<br>"
                << "<b>" << tr("CopyRight") << "</b>" << " : " << copyright
                << "</body></html>";
            rowItem->setToolTip(toolTip);
            result << rowItem;
        }
    }
    model->appendColumn(result);
}

PluginMetaObjectPointer PluginListView::currentPlugin() const
{
    QStandardItem *item = model->itemFromIndex(currentIndex());
    if (item) {
        QString pluginName = item->data(Qt::DisplayRole).toString();
        QString category = item->data(PluginListView::Category).toString();
        const QHash<QString, QQueue<PluginMetaObjectPointer>> &pluginCollections =
                LifeCycle::getPluginManagerInstance()->pluginCollections();
        auto &metaObjects = pluginCollections.value(category);
        for (auto metaObject : metaObjects) {
            if (metaObject->name() == pluginName)
                return metaObject;
        }
    }
    return {};
}
