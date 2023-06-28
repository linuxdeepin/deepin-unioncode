// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINVIEW_H
#define PLUGINVIEW_H

#include "framework/framework_global.h"
#include "framework/lifecycle/pluginmetaobject.h"

#include <QWidget>

class QTreeWidget;
class QStandardItemModel;

DPF_BEGIN_NAMESPACE

class PluginItem;
class CollectionItem;

class PluginView : public QWidget
{
    Q_OBJECT
public:
    explicit PluginView(QWidget *parent = nullptr);

    PluginMetaObjectPointer currentPlugin() const;

signals:
    void currentPluginChanged();
    void currentPluginActived(/*PluginMetaObjectPointer plugin*/);
    void pluginSettingChanged(PluginMetaObjectPointer plugin);

private:
    void updatePluginsWidegt();
    void setPluginEnabled(PluginMetaObjectPointer plugin, bool enable);

    QTreeWidget *categoryWidegt = nullptr;

    friend class PluginItem;
    friend class CollectionItem;
};

DPF_END_NAMESPACE

#endif // PLUGINVIEW_H
