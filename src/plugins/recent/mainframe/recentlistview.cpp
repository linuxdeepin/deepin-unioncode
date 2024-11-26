// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentlistview.h"
#include "itemdelegate.h"

#include <DFileIconProvider>

#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

static QIcon icon(const QString &file)
{
    static DFileIconProvider provider;
    return provider.icon(QFileInfo(file));
}

RecentListView::RecentListView(QWidget *parent)
    : DListView(parent)
{
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QListView::NoEditTriggers);
    setTextElideMode(Qt::ElideMiddle);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlternatingRowColors(false);
    setSelectionRectVisible(true);
    setMouseTracking(true);

    setUniformItemSizes(true);
    setResizeMode(Fixed);
    setOrientation(QListView::TopToBottom, false);
    setFrameStyle(QFrame::NoFrame);
    setSpacing(0);
    setContentsMargins(0, 0, 0, 0);

    setModel(&model);
    setItemDelegate(new ItemDelegate(this));
}

void RecentListView::setItemList(const QVariantList &items)
{
    for (const auto &item : items)
        appendItem(item);
}

bool RecentListView::contains(const QString &name) const
{
    auto list = model.findItems(name);
    return !list.isEmpty();
}

bool RecentListView::isEmpty() const
{
    return model.rowCount() == 0;
}

void RecentListView::clearAll()
{
    model.clear();
}

void RecentListView::clearProjects()
{
    for (int i = model.rowCount() - 1; i >= 0; --i) {
        const auto &item = model.item(i);
        if (item->data(IsProject).toBool())
            model.removeRow(i);
    }
}

void RecentListView::clearDocuments()
{
    for (int i = model.rowCount() - 1; i >= 0; --i) {
        const auto &item = model.item(i);
        if (!item->data(IsProject).toBool())
            model.removeRow(i);
    }
}

QVariantList RecentListView::projectList() const
{
    QVariantList itemList;
    for (int i = 0; i < model.rowCount(); ++i) {
        const auto &item = model.item(i);
        if (!item->data(IsProject).toBool())
            continue;

        QVariantMap map;
        map.insert(kKitName, item->data(KitNameRole));
        map.insert(kLanguage, item->data(LanguageRole));
        map.insert(kWorkspace, item->data(WorkspaceRole));

        itemList << map;
    }

    return itemList;
}

QVariantList RecentListView::documentList() const
{
    QVariantList itemList;
    for (int i = 0; i < model.rowCount(); ++i) {
        const auto &item = model.item(i);
        if (item->data(IsProject).toBool())
            continue;

        itemList << item->data(Qt::DisplayRole);
    }

    return itemList;
}

void RecentListView::appendItem(const QVariant &item)
{
    if (auto aitem = createItem(item)) {
        if (aitem->data(IsProject).toBool()) {
            int lastProjectRow = -1;
            for (int i = 0; i < model.rowCount(); ++i) {
                if (model.item(i)->data(IsProject).toBool()) {
                    lastProjectRow = i;
                    break;
                }
            }
            model.insertRow(lastProjectRow + 1, aitem);
        } else {
            model.appendRow(aitem);
        }
    }
}

void RecentListView::prependItem(const QVariant &item)
{
    if (auto aitem = createItem(item)) {
        if (aitem->data(IsProject).toBool()) {
            model.insertRow(0, aitem);
        } else {
            int firstDocRow = model.rowCount();
            for (int i = 0; i < model.rowCount(); ++i) {
                if (!model.item(i)->data(IsProject).toBool()) {
                    firstDocRow = i;
                    break;
                }
            }
            model.insertRow(firstDocRow, aitem);
        }
    }
}

DStandardItem *RecentListView::createItem(const QVariant &item)
{
    if (item.canConvert(QMetaType::QVariantMap))
        return createProjectItem(item);
    else if (item.canConvert(QMetaType::QString))
        return createDocumentItem(item);

    return nullptr;
}

DStandardItem *RecentListView::createProjectItem(const QVariant &item)
{
    const auto &infoMap = item.toMap();
    const auto kitName = infoMap.value(kKitName).toString();
    const auto language = infoMap.value(kLanguage).toString();
    const auto workspace = infoMap.value(kWorkspace).toString();

    auto rowItem = new DStandardItem(icon(workspace), workspace);
    rowItem->setData(kitName, KitNameRole);
    rowItem->setData(language, LanguageRole);
    rowItem->setData(workspace, WorkspaceRole);
    rowItem->setData(true, IsProject);
    QString tipFormat("KitName: %1\nLanguage: %2\nWorkspace: %3");
    rowItem->setToolTip(tipFormat.arg(kitName, language, workspace));

    return rowItem;
}

DStandardItem *RecentListView::createDocumentItem(const QVariant &item)
{
    QString filePath = item.toString();
    auto rowItem = new DStandardItem(icon(filePath), filePath);
    rowItem->setData(false, IsProject);
    return rowItem;
}

void RecentListView::focusOutEvent(QFocusEvent *e)
{
    clearSelection();
    DListView::focusOutEvent(e);
}

void RecentListView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;
    return DListView::mousePressEvent(e);
}
