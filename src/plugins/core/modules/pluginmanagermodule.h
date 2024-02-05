// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMANAGERMODULE_H
#define PLUGINMANAGERMODULE_H

#include "abstractmodule.h"

#include <QObject>

class PluginDialog;
class PluginManagerModule : public AbstractModule
{
    Q_OBJECT

public:
    PluginManagerModule();
    ~PluginManagerModule() override;

    virtual void initialize(Controller *uiController) override;

private:
    PluginDialog *pluginDialog = nullptr;
};

#endif   // PLUGINMANAGERMODULE_H
