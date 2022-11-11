/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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

    void initialize(const QString &buildDirectory);

    const QStringList getTargetNamesList() const;
    const QStringList getExeTargetNamesList() const;

    dpfservice::Target getActivedTargetByTargetType(const dpfservice::TargetType type);
    dpfservice::Target getTargetByName(const QString &targetName);

    const dpfservice::Targets getAllTargets() const;

    void updateActivedBuildTarget(const QString &targetName);
    void updateActivedCleanTarget(const QString &targetName);

    QString getCMakeConfigFile();

signals:
    void initialized();

public slots:

private:
    explicit TargetsManager(QObject *parent = nullptr);
    ~TargetsManager();

    bool isGloablTarget(dpfservice::Target &target);

    ProjectParser parser;

    dpfservice::Target buildTargetSelected;
    dpfservice::Target cleanTargetSelected;
    dpfservice::Target rebuildTargetSelected;

    dpfservice::Target exeTargetSelected;

    dpfservice::Targets exeTargets;
    dpfservice::Targets targets;

    QStringList buildTargetNameList;
    QStringList exeTargetNameList;
};

#endif // TARGETSMANAGER_H
