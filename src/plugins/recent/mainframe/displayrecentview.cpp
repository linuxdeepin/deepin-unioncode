// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "displayrecentview.h"
#include "common/common.h"
#include "displayitemdelegate.h"

#include <DStyledItemDelegate>
#include <DStandardItem>

#include <QToolTip>

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItemModel>

#define INIT_DATA "{\n    \"Projects\":[],\n    \"Documents\":[]\n}\n"

QJsonDocument DisplayRecentView::readRecent()
{
    QJsonDocument recentDoc;
    QFile file(cachePath());
    if (!file.exists()) {
        if (file.open(QFile::Truncate| QFile::WriteOnly)) {
            file.write(INIT_DATA);
            file.close();
        }
    } else {
        if (file.open(QFile::ReadOnly)) {
            recentDoc = QJsonDocument::fromJson(file.readAll());
            file.close();
        }

        if (!recentDoc.object().keys().contains("Projects")
                || !recentDoc.object().keys().contains("Documents")) {
            file.remove();
            file.close();
            file.open(QFile::Truncate| QFile::WriteOnly);
            file.write(INIT_DATA);
            file.close();
        }
    }

    if (file.open(QFile::ReadOnly)) {
        recentDoc = QJsonDocument::fromJson(file.readAll());
        file.close();
    }
    return recentDoc;
}

QList<QStandardItem *> DisplayRecentView::itemsFromFile()
{
    QJsonDocument doc = readRecent();
    QJsonObject obj = doc.object();
    QJsonArray array = obj.value(title()).toArray();
    QList<QStandardItem*> result;
    for (auto one : array) {
        QString filePath(one.toString());
        auto rowItem = new DStandardItem (icon(filePath), filePath);
        rowItem->setToolTip(filePath);
        if (!cache.contains(filePath)) {
            cache << filePath;
        }
        result << rowItem;
    }
    return result;
}

DisplayRecentView::DisplayRecentView(QWidget *parent)
    : DListView (parent)
    , cache({})
    , model(new QStandardItemModel(this))
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

    setModel(model);
    setItemDelegate(new DisplayItemDelegate(this));
}

DisplayRecentView::~DisplayRecentView()
{

}

void DisplayRecentView::focusOutEvent(QFocusEvent *e)
{
    DListView::clearSelection();

    DListView::focusOutEvent(e);
}

void DisplayRecentView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;
    return DListView::mousePressEvent(e);
}

QString DisplayRecentView::cachePath()
{
    return CustomPaths::user(CustomPaths::Configures)
            + QDir::separator() + "recent.support";
}

void DisplayRecentView::add(const QString &data)
{
    model->clear(); //删除数据
    if (cache.contains(data)) {
        cache.removeOne(data);
    }
    cache.insert(0, data); //置顶
    saveToFile(cache); //保存序列
    load(); //重新加载文件
}

QIcon DisplayRecentView::icon(const QString &data)
{
    QFileInfo info(data);
    return iconProvider.icon(info);
}

void DisplayRecentView::load()
{
    model->appendColumn(itemsFromFile());
}

void DisplayRecentView::saveToFile(const QStringList &cache)
{
    QJsonDocument doc = readRecent();
    QJsonObject docObj = doc.object();
    QJsonArray array;
    for (auto item : cache) {
        array << item;
    }
    docObj[title()] = array;
    doc.setObject(docObj);

    QFile file(cachePath());
    if (file.open(QFile::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}
