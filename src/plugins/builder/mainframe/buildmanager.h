// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUILDMANAGER_H
#define BUILDMANAGER_H

#include "services/builder/builderglobals.h"
#include "services/builder/task.h"
#include "services/project/projectinfo.h"
#include "common/widget/outputpane.h"

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

    CompileOutputPane *getCompileOutputPane() const;
    ProblemOutputPane *getProblemOutputPane() const;

    void setActivedProjectInfo(const QString &kitName, const QString &workingDir);
    void clearActivedProjectInfo();

    bool handleCommand(const QList<BuildCommandInfo> &info, bool isSynchronous);

signals:
    void sigOutputCompileInfo(const QString &content, const OutputPane::OutputFormat format);
    void sigOutputProblemInfo(const QString &content);
    void sigBuildState(const BuildState &buildstate);
    void sigOutputNotify(const BuildState &state, const BuildCommandInfo &commandInfo);
    void sigResetBuildUI();

public slots:
    void slotOutputCompileInfo(const QString &content, const OutputPane::OutputFormat format);
    void slotOutputProblemInfo(const QString &content);
    void addOutput(const QString &content, const OutputPane::OutputFormat format);
    void buildProject();
    void rebuildProject();
    void cleanProject();
    void cancelBuild();
    void slotBuildState(const BuildState &buildstate);
    void slotOutputNotify(const BuildState &state, const BuildCommandInfo &commandInfo);
    void slotResetBuildUI();

private:
    explicit BuildManager(QObject *parent = nullptr);
    virtual ~BuildManager();

    void addMenu();

    void outputLog(const QString &content, const OutputPane::OutputFormat format);
    void outputError(const QString &content);
    void outputNotify(const BuildState &state, const BuildCommandInfo &commandInfo);

    bool execCommands(const QList<BuildCommandInfo> &commandList, bool isSynchronous);
    bool execCommand(const BuildCommandInfo &info);

    void execBuildStep(QList<BuildMenuType> menuTypelist);
    void outBuildState(const BuildState &buildState);
    bool canStartBuild();
    void disconnectSignals();

    QMutex releaseMutex;

    BuildManagerPrivate *const d;
};

#endif // BUILDMANAGER_H

