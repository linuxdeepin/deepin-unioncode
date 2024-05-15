// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BINARYTOOLS_H
#define BINARYTOOLS_H

#include <framework/framework.h>

#include <QMenu>

class BinaryTools : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.unioncode" FILE "binarytools.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;

private:
    QMenu *toolMenu { nullptr };
};

#endif   // BINARYTOOLS_H
