// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kit.h"

#include <QUuid>

class KitPrivate
{
public:
    QString id;
    QString kitName;
    Option ccompiler;
    Option cxxcompiler;
    Option debugger;
    Option cmakeTool;
    QString cmakeGenerator;
};

Kit::Kit(QObject *parent)
    : QObject(parent),
      d(new KitPrivate())
{
}

Kit::Kit(const Kit &other)
    : d(new KitPrivate())
{
    copyFrom(other);
}

Kit::~Kit()
{
    delete d;
}

QString Kit::id() const
{
    return d->id;
}

void Kit::setId(const QString &id)
{
    d->id = id;
}

QString Kit::kitName() const
{
    return d->kitName;
}

void Kit::setKitName(const QString &name)
{
    d->kitName = name;
}

Option Kit::ccompiler() const
{
    return d->ccompiler;
}

void Kit::setCCompiler(const Option &opt)
{
    d->ccompiler = opt;
}

Option Kit::cxxcompiler() const
{
    return d->cxxcompiler;
}

void Kit::setCXXCompiler(const Option &opt)
{
    d->cxxcompiler = opt;
}

Option Kit::debugger() const
{
    return d->debugger;
}

void Kit::setDebugger(const Option &opt)
{
    d->debugger = opt;
}

Option Kit::cmakeTool() const
{
    return d->cmakeTool;
}

void Kit::setCMakeTool(const Option &opt)
{
    d->cmakeTool = opt;
}

QString Kit::cmakeGenerator() const
{
    return d->cmakeGenerator;
}

void Kit::setCMakeGenerator(const QString &cg)
{
    d->cmakeGenerator = cg;
}

bool Kit::isValid() const
{
    return !d->id.isEmpty();
}

void Kit::copyFrom(const Kit &other)
{
    d->id = other.d->id;
    d->kitName = other.d->kitName;
    d->ccompiler = other.d->ccompiler;
    d->cxxcompiler = other.d->cxxcompiler;
    d->debugger = other.d->debugger;
    d->cmakeTool = other.d->cmakeTool;
    d->cmakeGenerator = other.d->cmakeGenerator;
}

QVariantMap Kit::toVariantMap()
{
    return toVariantMap(*this);
}

QVariantMap Kit::toVariantMap(const Kit &kit)
{
    QMap<QString, QVariant> map;
    auto optionToMap = [&map](const QString key, const Option &opt) {
        QVariantMap optMap;
        optMap.insert(Name, opt.name);
        optMap.insert(Path, opt.path);
        map.insert(key, optMap);
    };

    optionToMap(kCCompiler, kit.d->ccompiler);
    optionToMap(kCXXCompiler, kit.d->cxxcompiler);
    optionToMap(kDebugger, kit.d->debugger);
    optionToMap(kCMakeTool, kit.d->cmakeTool);

    map.insert(kCMakeGenerator, kit.d->cmakeGenerator);
    map.insert(kID, kit.d->id);
    return map;
}

Kit Kit::fromVariantMap(const QVariantMap &map)
{
    auto updateKit = [map](const QString key, Option &opt) {
        const auto &item = map.value(key).toMap();
        opt.name = item.value(Name).toString();
        opt.path = item.value(Path).toString();
    };

    Kit kit;
    updateKit(kCCompiler, kit.d->ccompiler);
    updateKit(kCXXCompiler, kit.d->cxxcompiler);
    updateKit(kDebugger, kit.d->debugger);
    updateKit(kCMakeTool, kit.d->cmakeTool);

    kit.d->kitName = map.value(Name).toString();
    kit.d->cmakeGenerator = map.value(kCMakeGenerator).toString();
    kit.d->id = map.value(kID).toString();
    if (kit.d->id.isEmpty())
        kit.d->id = QUuid::createUuid().toString();

    return kit;
}

Kit &Kit::operator=(const Kit &other)
{
    copyFrom(other);
    return *this;
}

bool Kit::operator==(const Kit &other) const
{
    return d->id == other.d->id;
}
