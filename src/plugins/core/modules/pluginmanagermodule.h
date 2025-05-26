// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMANAGERMODULE_H
#define PLUGINMANAGERMODULE_H

#include "abstractmodule.h"

#include <QObject>

class PluginsUi;
class PluginManagerModule : public AbstractModule
{
    Q_OBJECT

public:
    PluginManagerModule();
    ~PluginManagerModule() override;

    virtual void initialize(Controller *uiController) override;

private:
    PluginsUi *pluginsUi = nullptr;
};

#endif   // PLUGINMANAGERMODULE_H
