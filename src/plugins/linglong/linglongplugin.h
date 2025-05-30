// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGLONG_PLUGIN_H
#define LINGLONG_PLUGIN_H

#include <framework/framework.h>

namespace dpfservice {
    class WindowService;
}

class LinglongPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "linglong.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

signals:

private:
    dpfservice::WindowService *windowService = nullptr;
};

#endif // LINGLONG_PLUGIN_H
