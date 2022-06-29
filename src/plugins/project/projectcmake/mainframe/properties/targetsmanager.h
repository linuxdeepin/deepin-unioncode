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
#ifndef TARGETSMANAGER_H
#define TARGETSMANAGER_H

#include "projectparser.h"

#include <QObject>

class TargetsManager : public QObject
{
    Q_OBJECT
public:
    static TargetsManager *instance();

    void intialize();

    dpfservice::Target getTarget(dpfservice::TargetType type);
    dpfservice::Target getSelectedTargetInList();
    dpfservice::Target getActiveBuildTarget();
    const dpfservice::Target &getActiveCleanTarget() const;
    const QStringList &getTargetNamesList() const;

    const dpfservice::Targets &getTargets() const;

    void updateActiveBuildTarget(const QString &target);
    void updateActiveCleanTarget(const QString &target);

    void save();

signals:

public slots:

private:
    explicit TargetsManager(QObject *parent = nullptr);
    bool isGloablTarget(dpfservice::Target &target);

    ProjectParser parser;
    QString activeCleanTargetName;

    dpfservice::Target buildTargetSelected;
    dpfservice::Target cleanTargetSelected;
    dpfservice::Target rebuildTargetSelected;

    dpfservice::Targets exeTargets;

    QStringList buildTargetList;
};

#endif // TARGETSMANAGER_H
