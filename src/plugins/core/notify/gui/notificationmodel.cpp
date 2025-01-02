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
    if (entity) {
        switch (role) {
        case Qt::DecorationRole: {
            auto type = entity->type();
            switch (type) {
            case NotificationEntity::Information:
                return QIcon::fromTheme("notification_info");
            case NotificationEntity::Warning:
                return QIcon::fromTheme("notification_warning");
            case NotificationEntity::Error:
                return QIcon::fromTheme("notification_error");
            }
        }
        case Qt::DisplayRole:
            return entity->message();
        case kActionsRole:
            return entity->actions();
        case kSourceRole: {
            QString source = entity->name();
            if (!source.isEmpty())
                return tr("Source: %1").arg(source);
        }
        case kEntityRole:
            return qVariantFromValue(entity);
        default:
            break;
        }
    }

    return QVariant();
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
