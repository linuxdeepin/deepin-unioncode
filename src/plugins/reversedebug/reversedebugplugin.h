// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REVERSEDEBUGPLUGIN_H
#define REVERSEDEBUGPLUGIN_H

#include <framework/framework.h>

namespace dpfservice {
class WindowService;
}

namespace ReverseDebugger {
namespace Internal {
class ReverseDebuggerMgr;
}
}

class ReverseDebugPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "reversedebug.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

private:
    dpfservice::WindowService *windowService = nullptr;
    ReverseDebugger::Internal::ReverseDebuggerMgr *reverseDebug = nullptr;
};

#endif   // REVERSEDEBUGPLUGIN_H
