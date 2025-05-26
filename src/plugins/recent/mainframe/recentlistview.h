// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECENTLISTVIEW_H
#define RECENTLISTVIEW_H

#include <DListView>
#include <DStandardItem>

#include <QStandardItemModel>

constexpr char kKitName[] { "KitName" };
constexpr char kLanguage[] { "Language" };
constexpr char kWorkspace[] { "Workspace" };

class RecentListView : public DTK_WIDGET_NAMESPACE::DListView
{
    Q_OBJECT
public:
    enum ItemRole {
        KitNameRole = Qt::ItemDataRole::UserRole,
        LanguageRole,
        WorkspaceRole,
        IsProject
    };

    RecentListView(QWidget *parent = nullptr);

    void setItemList(const QVariantList &items);
    QVariantList projectList() const;
    QVariantList documentList() const;
    bool contains(const QString &name) const;
    bool isEmpty() const;
    void clearAll();
    void clearProjects();
    void clearDocuments();
    void appendItem(const QVariant &item);
    void prependItem(const QVariant &item);

private:
    DTK_WIDGET_NAMESPACE::DStandardItem *createItem(const QVariant &item);
    DTK_WIDGET_NAMESPACE::DStandardItem *createProjectItem(const QVariant &item);
    DTK_WIDGET_NAMESPACE::DStandardItem *createDocumentItem(const QVariant &item);
    void focusOutEvent(QFocusEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

    QStandardItemModel model;
};

#endif   // RECENTLISTVIEW_H
