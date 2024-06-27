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
    explicit ProjectCoreReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

class ProjectProxy : public QObject
{
    Q_OBJECT
    ProjectProxy(){}
    ProjectProxy(const ProjectProxy&) = delete;

public:
    static ProjectProxy* instance()
    {
        static ProjectProxy ins;
        return &ins;
    }
signals:
    void projectActivated(const dpfservice::ProjectInfo prjInfo);
    void switchedFile(const QString &file);
};

#endif // PROJECTCORERECEIVER_H
