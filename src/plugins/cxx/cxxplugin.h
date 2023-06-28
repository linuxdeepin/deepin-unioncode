// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CXXPLUGIN_H
#define CXXPLUGIN_H

#include <framework/framework.h>

namespace dpfservice {
    class WindowService;
}

class CxxPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "cxxplugin.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

signals:

private:
    dpfservice::WindowService *windowService = nullptr;
};

#endif // CXXPLUGIN_H
