// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMARTUT_H
#define SMARTUT_H

#include <framework/framework.h>

#include <DToolButton>

class SmartUT : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "smartut.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;
    
private:    
    DTK_WIDGET_NAMESPACE::DToolButton *settingBtn { nullptr };
};

#endif   // SMARTUT_H
