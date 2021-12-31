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

#include <QObject>


class BuildStep;
class BuildOutputPane;
class BuildManager : public QObject
{
    Q_OBJECT
public:
    static BuildManager *instance();

    bool isBuilding() const;
    bool buildList(const QList<BuildStep*> &bsl);
    BuildOutputPane *getOutputPane() const;

    void destroy();

signals:

public slots:

private:
    explicit BuildManager(QObject *parent = nullptr);
    bool initBuildList(const QList<BuildStep*> &bsl);

    BuildOutputPane *outputPane = nullptr;

    bool isBuild = false;

};

#endif // BUILDMANAGER_H
