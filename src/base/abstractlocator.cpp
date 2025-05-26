// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractlocator.h"
#include "common/util/fuzzymatcher.h"

#include <QRegularExpression>

abstractLocator::abstractLocator(QObject *parent)
    : QObject(parent)
{
}

bool abstractLocator::isIncluedByDefault()
{
    return includedByDefault;
}

QRegularExpression abstractLocator::createRegExp(const QString &text)
{
    return FuzzyMatcher::createRegExp(text);
}

QString abstractLocator::getDisplayName()
{
    return displayName;
}

void abstractLocator::setDisplayName(const QString &displayName)
{
    this->displayName = displayName;
}

QString abstractLocator::getDescription()
{
    return description;
}

void abstractLocator::setDescription(const QString &description)
{
    this->description = description;
}

void abstractLocator::setIncludedDefault(bool isDefault)
{
    includedByDefault = isDefault;
}

QKeySequence abstractLocator::getShortCut()
{
    return this->shortCut;
}

void abstractLocator::setShortCut(const QKeySequence &key)
{
    this->shortCut = key;
}
