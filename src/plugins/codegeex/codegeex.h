// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CODEGEEX_H
#define CODEGEEX_H

#include <framework/framework.h>
#include <QObject>

class CodeGeex: public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "codegeex.json")
public:
    // Initialization function, executed in an asynchronous thread
    virtual void initialize() override;
    // Start the function and execute it in the main thread. If
    // there is an interface operation, please execute it within this function
    virtual bool start() override;
    // stop function
    virtual dpf::Plugin::ShutdownFlag stop() override;
};

#endif // CODEGEEX_H
