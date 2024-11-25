// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTCORERECEIVER_H
#define PROJECTCORERECEIVER_H

#include "common/project/projectinfo.h"
#include <framework/framework.h>

class ProjectCoreReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<ProjectCoreReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<ProjectCoreReceiver>;

public:
    explicit ProjectCoreReceiver(QObject *parent = nullptr);

    static Type type();
    static QStringList topics();
    virtual void eventProcess(const dpf::Event &event) override;

private:
    void processActiveProjectEvent(const dpf::Event &event);
    void processOpenProjectEvent(const dpf::Event &event);
    void processExpandAllEvent(const dpf::Event &event);
    void processFoldAllEvent(const dpf::Event &event);
    void processActivatedProjectEvent(const dpf::Event &event);
    void processSwitchedFileEvent(const dpf::Event &event);
    void processModeRaisedEvent(const dpf::Event &event);
    void processOpenProjectByPathEvent(const dpf::Event &event);
    void processSessionLoadedEvent(const dpf::Event &event);
    void processReadyToSaveSessionEvent(const dpf::Event &event);

private:
    QHash<QString, std::function<void(const dpf::Event &)>> eventHandleMap;
};

class ProjectProxy : public QObject
{
    Q_OBJECT
    ProjectProxy() {}
    ProjectProxy(const ProjectProxy &) = delete;

public:
    static ProjectProxy *instance()
    {
        static ProjectProxy ins;
        return &ins;
    }
signals:
    void modeRaised(const QString &mode);
    void openProject(const QString &path);
    void projectActivated(const dpfservice::ProjectInfo prjInfo);
    void switchedFile(const QString &file);
};

#endif   // PROJECTCORERECEIVER_H
