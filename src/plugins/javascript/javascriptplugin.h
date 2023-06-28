// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JAVASCRIPT_PLUGIN_H
#define JAVASCRIPT_PLUGIN_H

#include <framework/framework.h>

namespace dpfservice {
    class WindowService;
}

class JavascriptPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "javascriptplugin.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

signals:

private:
    dpfservice::WindowService *windowService = nullptr;
};

#endif // JAVASCRIPT_PLUGIN_H
