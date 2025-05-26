// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEPENDENCEMODULE_H
#define DEPENDENCEMODULE_H

#include "abstractmodule.h"
#include "depend/dependencemanager.h"

class DependenceModule : public AbstractModule
{
    Q_OBJECT
public:
    virtual void initialize(Controller *_uiController) override;

private:
    void initDefaultInstaller();
    void initInterfaces();
    void installPluginDepends();
    
    DependenceManager *manager { nullptr };
};

#endif   // DEPENDENCEMODULE_H
