/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef KITMANAGER_H
#define KITMANAGER_H

#include "kit.h"

#include <QObject>

class KitManager : public QObject
{
    Q_OBJECT
public:
    static KitManager *instance();
    ~KitManager() override;

    static QList<Kit *> kits();
    static Kit *kit(QString id);
    static Kit *defaultKit();

    static bool registerKit(std::unique_ptr<Kit> &&k);
    static void deregisterKit(Kit *k);
    static void setDefaultKit(Kit *k);

    static void saveKits();

    static bool isLoaded();

    void setSelectedKit(Kit &kit);
    const Kit &getSelectedKit();

    QString getDefaultOutputPath() const;

signals:

public slots:
private:
    explicit KitManager(QObject *parent = nullptr);

    void restoreKits();

    class KitList
    {
    public:
        KitList() {}
        QString defaultKit;
        std::vector<std::unique_ptr<Kit>> kits;
    };

    KitList restoreKits(const QString &fileName);
};

#endif // KITMANAGER_H
