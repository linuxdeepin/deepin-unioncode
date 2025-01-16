// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractllm.h"

AbstractLLM::AbstractLLM(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<ResponseState>("ResponseState");
}

void AbstractLLM::setModelState(LLMState st)
{
    if (state.loadAcquire() == st)
        return;

    state.storeRelease(st);
    Q_EMIT modelStateChanged();
}

AbstractLLM::LLMState AbstractLLM::modelState() const
{
    return static_cast<LLMState>(state.loadAcquire());
}
