// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef PLUGINSLIST_H
#define PLUGINSLIST_H

#include "framework/lifecycle/pluginmetaobject.h"

#include <DListView>

class QStandardItemModel;
class PluginListView : public DTK_WIDGET_NAMESPACE::DListView
{
    Q_OBJECT
public:
    enum ItemRole{
        Description = Qt::ItemDataRole::UserRole,
        Vender,
        Category
    };

    explicit PluginListView(QWidget *parent = nullptr);


    dpf::PluginMetaObjectPointer currentPlugin() const;
    void filter(const QString &filterText);
    void selectDefaultItem();

signals:
    void currentPluginActived();
    void pluginSettingChanged(dpf::PluginMetaObjectPointer plugin);

public slots:
    void display();

private:
    bool isFilterdOut(const QString &filteringText);
    bool isCorePlugin(const QString &category);

    QStandardItemModel *model = nullptr;
    QString filterText;
};

#endif // PLUGINSLIST_H
