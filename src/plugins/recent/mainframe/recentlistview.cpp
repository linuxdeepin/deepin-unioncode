// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentlistview.h"

#include <DFileIconProvider>

static QIcon icon(const QString &file)
{
    static DFileIconProvider provider;
    return provider.icon(QFileInfo(file));
}

RecentProjectView::RecentProjectView(QWidget *parent)
    : ItemListView(parent)
{
}

QString RecentProjectView::title()
{
    return tr("Projects");
}

QString RecentProjectView::configKey()
{
    return "Projects";
}

void RecentProjectView::setItemList(const QVariantList &items)
{
    clear();
    for (const auto &item : items) {
        appendItem(item);
    }
}

QVariantList RecentProjectView::itemList() const
{
    QVariantList itemList;
    for (int i = 0; i < model->rowCount(); ++i) {
        QVariantMap map;
        const auto &item = model->item(i);
        map.insert(kKitName, item->data(KitNameRole));
        map.insert(kLanguage, item->data(LanguageRole));
        map.insert(kWorkspace, item->data(WorkspaceRole));

        itemList << map;
    }

    return itemList;
}

void RecentProjectView::appendItem(const QVariant &item)
{
    if (!item.canConvert(QMetaType::QVariantMap))
        return;

    model->appendRow(createItem(item));
}

void RecentProjectView::prependItem(const QVariant &item)
{
    if (!item.canConvert(QMetaType::QVariantMap))
        return;

    model->insertRow(0, createItem(item));
}

DStandardItem *RecentProjectView::createItem(const QVariant &item)
{
    const auto &infoMap = item.toMap();
    const auto kitName = infoMap.value(kKitName).toString();
    const auto language = infoMap.value(kLanguage).toString();
    const auto workspace = infoMap.value(kWorkspace).toString();

    auto rowItem = new DStandardItem(icon(workspace), workspace);
    rowItem->setData(kitName, KitNameRole);
    rowItem->setData(language, LanguageRole);
    rowItem->setData(workspace, WorkspaceRole);
    QString tipFormat("KitName: %1\nLanguage: %2\nWorkspace: %3");
    rowItem->setToolTip(tipFormat.arg(kitName, language, workspace));

    return rowItem;
}

RecentDocemntView::RecentDocemntView(QWidget *parent)
    : ItemListView(parent)
{
}

QString RecentDocemntView::title()
{
    return tr("Documents");
}

QString RecentDocemntView::configKey()
{
    return "Documents";
}

void RecentDocemntView::setItemList(const QVariantList &items)
{
    clear();
    for (const auto &item : items) {
        appendItem(item);
    }
}

QVariantList RecentDocemntView::itemList() const
{
    QVariantList itemList;
    for (int i = 0; i < model->rowCount(); ++i) {
        const auto &item = model->item(i);
        const auto filePath = item->data(Qt::DisplayRole);
        itemList << filePath;
    }

    return itemList;
}

void RecentDocemntView::appendItem(const QVariant &item)
{
    if (!item.canConvert(QMetaType::QString))
        return;

    QString filePath = item.toString();
    auto rowItem = new DStandardItem(icon(filePath), filePath);
    model->appendRow(rowItem);
}

void RecentDocemntView::prependItem(const QVariant &item)
{
    if (!item.canConvert(QMetaType::QString))
        return;

    QString filePath = item.toString();
    auto rowItem = new DStandardItem(icon(filePath), filePath);
    model->insertRow(0, rowItem);
}
