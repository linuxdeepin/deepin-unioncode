// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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

QString abstractLocator::getShortcut()
{
    return shortcut;
}

void abstractLocator::setShortcut(const QString &shortcut)
{
    this->shortcut = shortcut;
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
