// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUILDERSENDER_H
#define BUILDERSENDER_H

#include "services/builder/builderglobals.h"

#include <QObject>

class BuilderSender final : QObject
{
    Q_OBJECT
public:
    static void notifyBuildState(BuildState state, const BuildCommandInfo &commandInfo);

private:
    explicit BuilderSender(QObject *parent = nullptr);
    virtual ~BuilderSender();
};

#endif   // EVENTSENDER_H
