// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTGRADLERECEIVER_H
#define PROJECTGRADLERECEIVER_H

#include "common/project/projectinfo.h"
#include "services/builder/builderglobals.h"

#include <framework/framework.h>

class ProjectGradleReceiver : public dpf::EventHandler,
        dpf::AutoEventHandlerRegister<ProjectGradleReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<ProjectGradleReceiver>;
public:
    explicit ProjectGradleReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

class ProjectGradleProxy : public QObject
{
    Q_OBJECT
    ProjectGradleProxy(){}
    ProjectGradleProxy(const ProjectGradleProxy&) = delete;

public:
    static ProjectGradleProxy* instance();

signals:
    void nodeExpanded(const QString &filePath);
    void nodeCollapsed(const QString &filePath);

};

#endif // PROJECTGRADLERECEIVER_H
