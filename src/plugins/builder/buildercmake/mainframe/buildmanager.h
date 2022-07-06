/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#ifndef BUILDMANAGER_H
#define BUILDMANAGER_H

#include "buildstep.h"
#include "services/project/projectservice.h"
#include "services/builder/builderglobals.h"

#include <QObject>
#include <QSharedPointer>
#include <QMutex>

class Project;
namespace dpfservice {
class WindowService;
}

class BuildManager : public QObject
{
    Q_OBJECT
public:

    static BuildManager *instance();

    bool buildList(const QList<BuildStep*> &bsl, QString originCmd = "");
    BuildStep *makeCommandStep(const QString &cmd, const QStringList &args, QString outputDirectory = "");

    void destroy();

signals:

public slots:
    void buildProject();
    void rebuildProject();
    void cleanProject();

private:
    explicit BuildManager(QObject *parent = nullptr);
    ~BuildManager();
    bool initBuildList(const QList<BuildStep*> &_bsl);

    void initialize();

    BuildStep *makeBuildStep();
    BuildStep *makeCleanStep();
    BuildStep *makeStep(dpfservice::TargetType type);

    QList<BuildStep *> bsl;

    QSharedPointer<Project> project;
    BuildState buildState = kNoBuild;

    QMutex releaseMutex;
};

#endif // BUILDMANAGER_H
