// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTEXTMODULE_H
#define CONTEXTMODULE_H

#include "abstractmodule.h"

class ContextModule : public AbstractModule
{
    Q_OBJECT
public:
    virtual void initialize(Controller *_uiController) override;
};

#endif   // CONTEXTMODULE_H
