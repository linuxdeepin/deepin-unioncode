// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customicons.h"
#include <QIcon>
#include <QMimeDatabase>

namespace {
QFileIconProvider provider;
QMimeDatabase mimeDatabase;
}

QIcon CustomIcons::icon(QFileIconProvider::IconType type)
{
    return provider.icon(type);
}

QIcon CustomIcons::icon(CustomIcons::CustomIconType type)
{
    switch (type) {
    case CustomIcons::Exe:
        return QIcon::fromTheme("application/x-executable");
    case CustomIcons::Lib:
        return QIcon::fromTheme("application/x-sharedlib");
    }
    return {};
}

QIcon CustomIcons::icon(const QFileInfo &info)
{
    return provider.icon(info);
}

QString CustomIcons::type(const QFileInfo &info)
{
    return provider.type(info);
}

void CustomIcons::setOptions(QFileIconProvider::Options options)
{
    return provider.setOptions(options);
}

QFileIconProvider::Options CustomIcons::options()
{
    return provider.options();
}
