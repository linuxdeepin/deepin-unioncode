// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buildersender.h"
#include "framework/framework.h"
#include "common/common.h"

BuilderSender::BuilderSender(QObject *parent)
    : QObject(parent)
{

}

BuilderSender::~BuilderSender()
{

}

void BuilderSender::notifyBuildState(BuildState state, const BuildCommandInfo &commandInfo)
{
    dpf::Event event;
    event.setTopic(T_BUILDER);
    event.setData(D_BUILD_STATE);
    event.setProperty(P_STATE, static_cast<int>(state));
    event.setProperty(P_ORIGINCMD, QVariant::fromValue(commandInfo));
    dpf::EventCallProxy::instance().pubEvent(event);
}

