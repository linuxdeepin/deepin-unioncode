/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
