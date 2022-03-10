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

#include <QObject>
#include <QSharedPointer>
#include <QMutex>

class BuildOutputPane;
class Project;
class MenuManager;
namespace dpfservice {
class WindowService;
}

class BuildManager : public QObject
{
    Q_OBJECT
public:

    enum BuildState
    {
        kNoBuild,
        kBuilding,
    };

    static BuildManager *instance();

    void initialize(dpfservice::WindowService *service);

    bool buildList(const QList<BuildStep*> &bsl);
    BuildOutputPane *getOutputPane() const;

    void destroy();

signals:

public slots:
    void slotOutput(const QString &content, BuildStep::OutputFormat format);
    void buildProject();

private:
    explicit BuildManager(QObject *parent = nullptr);
    ~BuildManager();
    bool initBuildList(const QList<BuildStep*> &_bsl);

    BuildOutputPane *outputPane = nullptr;

    QList<BuildStep *> bsl;

    QSharedPointer<Project> project;
    QSharedPointer<MenuManager> menuManager;
    BuildState buildState = kNoBuild;

    QMutex releaseMutex;
};

#endif // BUILDMANAGER_H
