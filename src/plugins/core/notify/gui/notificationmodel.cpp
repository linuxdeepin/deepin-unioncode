// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notificationmodel.h"
#include "notificationlistview.h"

class NotificationModelPrivate
{
public:
    explicit NotificationModelPrivate(NotificationModel *qq);

    EntityPtr getEntityByRow(int row);

public:
    NotificationModel *q;
    NotificationListView *view { nullptr };
    QList<EntityPtr> notifications;
};

NotificationModelPrivate::NotificationModelPrivate(NotificationModel *qq)
    : q(qq)
{
}

EntityPtr NotificationModelPrivate::getEntityByRow(int row)
{
    Q_ASSERT(row <= q->rowCount(QModelIndex()) - 1);
    return notifications[row];
}

NotificationModel::NotificationModel(QObject *parent, NotificationListView *view)
    : QAbstractListModel(parent),
      d(new NotificationModelPrivate(this))
{
    d->view = view;
}

NotificationModel::~NotificationModel()
{
    delete d;
}

int NotificationModel::rowCount(const QModelIndex &parent) const
{
    return d->notifications.size();
}

QVariant NotificationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() > (rowCount(QModelIndex()) - 1)) {
        return QVariant();
    }

    EntityPtr entity = d->getEntityByRow(index.row());
    if (entity)
        return QVariant::fromValue(entity);

    return QVariant();
}

Qt::ItemFlags NotificationModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (d->view)
            d->view->openPersistentEditor(index);
        return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractListModel::flags(index);
}

void NotificationModel::setNotifications(const QList<EntityPtr> &datas)
{
    beginResetModel();
    d->notifications = datas;
    endResetModel();
}

void NotificationModel::addNotification(EntityPtr entity)
{
    beginResetModel();
    d->notifications.push_front(entity);
    endResetModel();
}

void NotificationModel::removeNotification(EntityPtr entity)
{
    if (!d->notifications.contains(entity))
        return;

    beginResetModel();
    d->notifications.removeOne(entity);
    endResetModel();
}

void NotificationModel::clear()
{
    beginResetModel();
    d->notifications.clear();
    endResetModel();
}
