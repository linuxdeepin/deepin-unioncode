// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONMANAGERMODULE_H
#define SESSIONMANAGERMODULE_H

#include "abstractmodule.h"

class SessionManagerModule : public AbstractModule
{
    Q_OBJECT
public:
    virtual void initialize(Controller *_uiController) override;

private:
    void initInterfaces();
    void initOutputEvents();
};

#endif   // SESSIONMANAGERMODULE_H
