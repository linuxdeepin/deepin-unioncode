// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFICATIONMODEL_H
#define NOTIFICATIONMODEL_H

#include "notify/constants.h"

#include <QAbstractListModel>

class NotificationListView;
class NotificationModelPrivate;
class NotificationModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit NotificationModel(QObject *parent = nullptr, NotificationListView *view = nullptr);
    ~NotificationModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

public Q_SLOTS:
    void setNotifications(const QList<EntityPtr> &datas);
    void addNotification(EntityPtr entity);
    void removeNotification(EntityPtr entity);
    void clear();

private:
    NotificationModelPrivate *const d;
};

#endif   // NOTIFICATIONMODEL_H
