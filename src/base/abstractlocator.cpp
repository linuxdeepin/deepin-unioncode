// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractlocator.h"
#include "common/util/fuzzymatcher.h"

#include <QRegularExpression>

class AbstractLocatorPrivate
{
public:
    explicit AbstractLocatorPrivate() {} 
    bool includedByDefault = true;
    QString displayName = "";
    QString description = "";
    QKeySequence shortCut{};
};

// Constructor
abstractLocator::abstractLocator(QObject *parent)
    : QObject(parent)
    , d(new AbstractLocatorPrivate)
{
}

// Destructor
abstractLocator::~abstractLocator()
{
    delete d;
}

bool abstractLocator::isIncluedByDefault() const // Corrected typo in method name
{
    return d->includedByDefault;
}

QRegularExpression abstractLocator::createRegExp(const QString &text) const
{
    return FuzzyMatcher::createRegExp(text);
}

QString abstractLocator::getDisplayName() const
{
    return d->displayName;
}

void abstractLocator::setDisplayName(const QString &displayName)
{
    d->displayName = displayName;
}

QString abstractLocator::getDescription() const
{
    return d->description;
}

void abstractLocator::setDescription(const QString &description)
{
    d->description = description;
}

void abstractLocator::setIncludedDefault(bool isDefault)
{
    d->includedByDefault = isDefault; // Using d-pointer
}

QKeySequence abstractLocator::getShortCut() const
{
    return d->shortCut; // Using d-pointer
}

void abstractLocator::setShortCut(const QKeySequence &key)
{
    d->shortCut = key;
}

