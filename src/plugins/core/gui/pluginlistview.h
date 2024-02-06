// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef PLUGINSLIST_H
#define PLUGINSLIST_H

#include "framework/lifecycle/pluginmetaobject.h"

#include <QListView>

class QStandardItemModel;
class PluginListView : public QListView
{
    Q_OBJECT
public:
    enum ItemRole{
        Description = Qt::ItemDataRole::UserRole,
        Vender,
        Category
    };

    explicit PluginListView(QListView *parent = nullptr);


    dpf::PluginMetaObjectPointer currentPlugin() const;

signals:
    void currentPluginActived();
    void pluginSettingChanged(dpf::PluginMetaObjectPointer plugin);

public slots:
    void display();

private:
    QStandardItemModel *model = nullptr;
};

#endif // PLUGINSLIST_H
