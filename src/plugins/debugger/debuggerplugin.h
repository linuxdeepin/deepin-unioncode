// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBUGGERPLUGIN_H
#define DEBUGGERPLUGIN_H

#include <framework/framework.h>

namespace dpfservice {
class WindowService;
}
class DebuggerPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "debugger.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

public slots:
    void slotDebugStarted();

private:
    dpfservice::WindowService *windowService = nullptr;
};

#endif // DEBUGGERPLUGIN_H
