/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "kit.h"

#include <QUuid>

const char ID_KEY[] = "PE.Profile.Id";
const char DISPLAYNAME_KEY[] = "PE.Profile.Name";
const char DATA_KEY[] = "PE.Profile.Data";

class KitPrivate
{
public:
    explicit KitPrivate(QString _id)
        : id(_id)
    {
        if (id.isEmpty())
            id = QUuid::createUuid().toString();
    }

    QString id;
    QString unexpandedDisplayName;

    QHash<QString, QVariant> data;

    QString defaultOutput;
};

Kit::Kit(QString id, QObject *parent) : QObject(parent)
  , d(std::make_unique<KitPrivate>(id))
{
}

QString fromSetting(const QVariant &variant)
{
    const QByteArray ba = variant.toString().toUtf8();
    if (ba.isEmpty())
        return {};
    return ba;
}

QString fromString(const QString &name)
{
    if (name.isEmpty())
        return {};
    return name.toUtf8();
}

Kit::Kit(const QVariantMap &data)
    : d(std::make_unique<KitPrivate>(""))
{
    d->id = fromSetting(data.value(QLatin1String(ID_KEY)));

    d->unexpandedDisplayName = data.value(QLatin1String(DISPLAYNAME_KEY),
                                            d->unexpandedDisplayName).toString();

    QVariantMap extra = data.value(QLatin1String(DATA_KEY)).toMap();
    d->data.clear(); // remove default values
    const QVariantMap::ConstIterator cend = extra.constEnd();
    for (QVariantMap::ConstIterator it = extra.constBegin(); it != cend; ++it)
        d->data.insert(fromString(it.key()), it.value());
}

QString Kit::displayName() const
{
    return d->unexpandedDisplayName;
}

void Kit::setUnexpandedDisplayName(const QString &name)
{
    if (d->unexpandedDisplayName == name)
        return;

    d->unexpandedDisplayName = name;
}

QString Kit::id() const
{
    return d->id;
}

QList<QString> Kit::allKeys() const
{
    return d->data.keys();
}

QVariant Kit::value(QString key, const QVariant &unset) const
{
    return d->data.value(key, unset);
}

bool Kit::hasValue(QString key) const
{
    return d->data.contains(key);
}

void Kit::setValue(QString key, const QVariant &value)
{
    if (d->data.value(key) == value)
        return;
    d->data.insert(key, value);
}

void Kit::setDefaultOutput(QString &defaultOutput)
{
    d->defaultOutput = defaultOutput;
}

const QString &Kit::getDefaultOutput() const
{
    return d->defaultOutput;
}

void Kit::copyFrom(const Kit &k)
{
    d->data = k.d->data;
    d->unexpandedDisplayName = k.d->unexpandedDisplayName;
    d->defaultOutput = k.d->defaultOutput;
}

Kit::Kit(const Kit &other)
{
    copyFrom(other);
}

Kit &Kit::operator=(const Kit &other)
{
    copyFrom(other);
    return *this;
}

QVariantMap Kit::toMap() const
{
    using IdVariantConstIt = QHash<QString, QVariant>::ConstIterator;

    QVariantMap data;
    data.insert(QLatin1String(ID_KEY), d->id);
    data.insert(QLatin1String(DISPLAYNAME_KEY), d->unexpandedDisplayName);

    QVariantMap extra;

    const IdVariantConstIt cend = d->data.constEnd();
    for (IdVariantConstIt it = d->data.constBegin(); it != cend; ++it)
        extra.insert(it.key(), it.value());
    data.insert(QLatin1String(DATA_KEY), extra);

    return data;
}

Kit::~Kit() = default;
