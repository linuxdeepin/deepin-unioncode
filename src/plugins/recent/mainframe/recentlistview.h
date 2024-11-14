// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTLISTVIEW_H
#define RECENTLISTVIEW_H

#include "itemlistview.h"

constexpr char kKitName[] { "KitName" };
constexpr char kLanguage[] { "Language" };
constexpr char kWorkspace[] { "Workspace" };

class RecentProjectView : public ItemListView
{
    Q_OBJECT
public:
    enum ItemRole {
        KitNameRole = Qt::ItemDataRole::UserRole,
        LanguageRole,
        WorkspaceRole
    };

    RecentProjectView(QWidget *parent = nullptr);

    QString title() override;
    QString configKey() override;
    void setItemList(const QVariantList &items) override;
    QVariantList itemList() const override;
    void appendItem(const QVariant &item) override;
    void prependItem(const QVariant &item) override;

private:
    DStandardItem *createItem(const QVariant &item);
};

class RecentDocemntView : public ItemListView
{
    Q_OBJECT
public:
    RecentDocemntView(QWidget *parent = nullptr);

    QString title() override;
    QString configKey() override;
    void setItemList(const QVariantList &items) override;
    QVariantList itemList() const override;
    void appendItem(const QVariant &item) override;
    void prependItem(const QVariant &item) override;
};

#endif   // RECENTLISTVIEW_H
