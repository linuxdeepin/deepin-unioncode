// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "pluginlistview.h"
#include "framework/lifecycle/lifecycle.h"
#include "pluginitemdelegate.h"
#include "common/util/fuzzymatcher.h"
#include "common/util/custompaths.h"

#include <DStandardItem>
#include <DIconTheme>

#include <QStandardItemModel>
#include <QDir>

DPF_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

PluginListView::PluginListView(QWidget *parent)
    : DListView(parent)
    , model(new QStandardItemModel(this))
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    setModel(model);
    setItemDelegate(new PluginItemDelegate(this));

    QObject::connect(LifeCycle::getPluginManagerInstance(), &PluginManager::pluginsChanged, this,
                     &PluginListView::display);

    QObject::connect(this, &QListView::clicked, [this](){
            emit currentPluginActived();
    });

    display();

    selectDefaultItem();
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
            QString category = metaOBject->category();
            if (isCorePlugin(category) || isFilterdOut(pluginName))
                continue;

            QString description = metaOBject->description();
            QString vender = metaOBject->vendor();
            QString copyright = metaOBject->copyright();
            QStringList license = metaOBject->license();
            QString version = metaOBject->version();

            QString pluginPath = CustomPaths::global(CustomPaths::Plugins);
            QString pluginLogoPath = pluginPath + QDir::separator() + pluginName + ".svg";
            QIcon pluginLogo;
            if (QFile::exists(pluginLogoPath)) {
                pluginLogo = DIconTheme::findQIcon(pluginLogoPath);
            } else {
                pluginLogo = DIconTheme::findQIcon("default_plugin");
            }

            auto rowItem = new DStandardItem(pluginLogo, pluginName);
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

bool PluginListView::isFilterdOut(const QString &filteringText)
{
    bool filterOut = false;
    if (!filterText.isEmpty()) {
        QRegularExpression regexp = FuzzyMatcher::createRegExp(filterText);
        QRegularExpressionMatch match = regexp.match(filteringText);
        if (!match.hasMatch()) {
            filterOut = true;
        }
    }
    return filterOut;
}

bool PluginListView::isCorePlugin(const QString &category)
{
    if (category.toLower() == QString("Core Plugins").toLower())
        return true;
    return false;
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

void PluginListView::filter(const QString &filterText)
{
    this->filterText = filterText;
    display();
}

void PluginListView::selectDefaultItem()
{
    setCurrentIndex(model->index(0, 0));
}
