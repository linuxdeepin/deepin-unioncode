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
