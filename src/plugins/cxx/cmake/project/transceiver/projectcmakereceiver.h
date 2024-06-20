// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTRECEIVER_H
#define PROJECTRECEIVER_H

#include "common/project/projectinfo.h"
#include "services/builder/builderglobals.h"

#include <framework/framework.h>

class ProjectCmakeReceiver : public dpf::EventHandler,
        dpf::AutoEventHandlerRegister<ProjectCmakeReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<ProjectCmakeReceiver>;
public:
    explicit ProjectCmakeReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;

    virtual void builderEvent(const dpf::Event& event);
};

class ProjectCmakeProxy : public QObject
{
    Q_OBJECT
    ProjectCmakeProxy(){}
    ProjectCmakeProxy(const ProjectCmakeProxy&) = delete;

public:
    static ProjectCmakeProxy* instance();

    void setBuildCommandUuid(QString buildCommandUuid);
    QString getBuildCommandUuid();

signals:
    void buildExecuteEnd(const BuildCommandInfo &commandInfo, int status = 0);
    void fileDeleted(const QString &filePath);

private:
    QString buildCommandUuid;
};

#endif // PROJECTRECEIVER_H
