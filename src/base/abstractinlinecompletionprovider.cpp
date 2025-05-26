// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractinlinecompletionprovider.h"

AbstractInlineCompletionProvider::AbstractInlineCompletionProvider(QObject *parent)
    : QObject(parent)
{
}

bool AbstractInlineCompletionProvider::inlineCompletionEnabled() const
{
    return false;
}

void AbstractInlineCompletionProvider::accepted()
{
}

void AbstractInlineCompletionProvider::rejected()
{
}
