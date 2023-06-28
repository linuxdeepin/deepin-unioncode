// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEPORTINGPLUGIN_H
#define CODEPORTINGPLUGIN_H

#include <framework/framework.h>

namespace dpfservice {
class WindowService;
}
class CodePortingPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "codeporting.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

private:
    dpfservice::WindowService *windowService = nullptr;
};

#endif   // CODEPORTINGPLUGIN_H
