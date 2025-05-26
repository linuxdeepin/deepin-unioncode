// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEMPLATEPLUGIN_H
#define TEMPLATEPLUGIN_H

#include <framework/framework.h>

class TemplatePlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "templateplugin.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

signals:

private:
};

#endif // TEMPLATEPLUGIN_H
