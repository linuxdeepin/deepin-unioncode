/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef BUILDTARGET_H
#define BUILDTARGET_H

#include <QObject>

/**
 * @brief Will be refactored in the future.
 */
class BuildStep;
class Project;
class BuildTarget : public QObject
{
    Q_OBJECT
public:
    BuildTarget(Project *pro);
    ~BuildTarget();

    QString buildOutputDirectory() const;
    QList<BuildStep *> &getbuildSteps();

private:
    bool constructBuildSteps();
    void clear();

    QList<BuildStep *> buildSteps;
    Project *project;

signals:

public slots:
};

#endif // BUILDTARGET_H
