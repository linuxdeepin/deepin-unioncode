// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PYTHONPLUGIN_H
#define PYTHONPLUGIN_H

#include <framework/framework.h>

class PythonPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "pythonplugin.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

private:
    void registEditorService();
    void registerPIPInstaller();
};

#endif // PYTHONPLUGIN_H
