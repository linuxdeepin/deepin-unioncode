// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractdocumentfind.h"

AbstractDocumentFind::AbstractDocumentFind(QObject *parent)
    : QObject(parent)
{
}

void AbstractDocumentFind::replace(const QString &before, const QString &after)
{
    Q_UNUSED(before)
    Q_UNUSED(after)
}

void AbstractDocumentFind::replaceFind(const QString &before, const QString &after)
{
    Q_UNUSED(before)
    Q_UNUSED(after)
}

void AbstractDocumentFind::replaceAll(const QString &before, const QString &after)
{
    Q_UNUSED(before)
    Q_UNUSED(after)
}

void AbstractDocumentFind::findStringChanged()
{
}

bool AbstractDocumentFind::supportsReplace() const
{
    return true;
}
