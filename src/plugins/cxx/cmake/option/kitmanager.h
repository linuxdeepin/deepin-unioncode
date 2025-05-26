// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KITMANAGER_H
#define KITMANAGER_H

#include "kit.h"

#include <QObject>

class KitManager : public QObject
{
    Q_OBJECT
public:
    static KitManager *instance();

    void setKitList(const QList<Kit> &list);
    void removeKit(const Kit &kit);
    QList<Kit> kitList() const;
    Kit findKit(const QString &id);
    Kit defaultKit() const;

private:
    explicit KitManager(QObject *parent = nullptr);

    QList<Kit> allKit;
};

#endif   // KITMANAGER_H
