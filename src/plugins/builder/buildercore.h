// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUILDERPLUGIN_H
#define BUILDERPLUGIN_H

#include <framework/framework.h>

class Project;
namespace dpfservice {
class WindowService;
}
class BuilderCore : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "buildercore.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

public slots:
private:
};

#endif // BUILDERPLUGIN_H
