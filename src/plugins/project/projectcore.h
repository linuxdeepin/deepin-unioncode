// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROJECTCORE_H
#define PROJECTCORE_H

#include "services/window/windowservice.h"

#include <DToolButton>

#include <framework/framework.h>

class ProjectCore : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "projectcore.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

    void addRecentOpenWidget(dpfservice::WindowService *windowservice);
    void addAutoFocusSwitcher(dpfservice::WindowService *windowService, 
        Dtk::Widget::DToolButton *autoFocusSwitcher, Dtk::Widget::DToolButton *focusFile);
    void addProjectProperty(dpfservice::WindowService *windowService, 
        Dtk::Widget::DToolButton *projectProperty);

    void initLocator(dpf::PluginServiceContext& ctx);
    void initProject(dpf::PluginServiceContext& ctx);

private slots:
    void pluginsStartedMain();
};

#endif // PROJECTCORE_H
