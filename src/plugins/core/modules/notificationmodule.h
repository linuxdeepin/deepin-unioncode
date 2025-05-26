// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFICATIONMODULE_H
#define NOTIFICATIONMODULE_H

#include "abstractmodule.h"

class NotificationModule : public AbstractModule
{
    Q_OBJECT
public:
    virtual void initialize(Controller *_uiController) override;
};

#endif // NOTIFICATIONMODULE_H
