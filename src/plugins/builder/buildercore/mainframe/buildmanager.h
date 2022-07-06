/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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

#include "services/builder/builderglobals.h"
#include "services/builder/task.h"

#include <QObject>
#include <QSharedPointer>
#include <QMutex>

namespace dpfservice {
class WindowService;
}

class CompileOutputPane;
class ProblemOutputPane;
class BuildManagerPrivate;
class BuildManager : public QObject
{
    Q_OBJECT
public:
    static BuildManager *instance();
    void initialize(dpfservice::WindowService *service);

    CompileOutputPane *getCompileOutputPane() const;
    ProblemOutputPane *getProblemOutputPane() const;

    void outputCompileInfo(const QString &content, OutputFormat format);
    void outputProblemInfo(const Task &task, int linkedOutputLines, int skipLines);

    void dispatchCommand(const QString &program, const QStringList &arguments, const QString &workingDir);
    void buildStateChanged(BuildState state, QString originCmd);

signals:
    void buildStarted();

public slots:
    void buildActivedProject();
    void rebuildActivedProject();
    void cleanActivedProject();

private:
    explicit BuildManager(QObject *parent = nullptr);
    virtual ~BuildManager();

    void startBuild();

    BuildManagerPrivate *const d;
};

#endif // BUILDMANAGER_H

