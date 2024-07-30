// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kitmanager.h"

KitManager::KitManager(QObject *parent)
    : QObject(parent)
{
}

KitManager *KitManager::instance()
{
    static KitManager ins;
    return &ins;
}

void KitManager::setKitList(const QList<Kit> &list)
{
    allKit = list;
}

void KitManager::removeKit(const Kit &kit)
{
    allKit.removeOne(kit);
}

QList<Kit> KitManager::kitList() const
{
    return allKit;
}

Kit KitManager::findKit(const QString &id)
{
    auto iter = std::find_if(allKit.cbegin(), allKit.cend(),
                             [&id](const Kit &kit) {
                                 return kit.id() == id;
                             });

    return iter != allKit.cend() ? *iter : Kit();
}
