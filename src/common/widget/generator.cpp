// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "generator.h"

#include <QStandardItem>

Generator::Generator(QObject *parent)
    : QObject(parent)
{

}

QString Generator::errorString()
{
    return property("errorString").toString();
}

bool Generator::setErrorString(const QString &error)
{
    return setProperty("errorString", error);
}

bool Generator::setProperty(const QString &name, const QVariant &value)
{
    return QObject::setProperty(name.toLatin1(), value);
}

QVariant Generator::property(const QString &name) const
{
    return QObject::property(name.toLatin1());
}
