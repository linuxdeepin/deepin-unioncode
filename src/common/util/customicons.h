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
#ifndef CUSTOMICON_H
#define CUSTOMICON_H

#include <QFileIconProvider>

class CustomIcons final
{
    CustomIcons() = delete;
    Q_DISABLE_COPY(CustomIcons)
public:
    enum CustomIconType { Exe, Lib };
    static QIcon icon(QFileIconProvider::IconType type);
    static QIcon icon(CustomIconType type);
    static QIcon icon(const QFileInfo &info);
    static QString type(const QFileInfo &info);
    static void setOptions(QFileIconProvider::Options options);
    static QFileIconProvider::Options options();
};

#endif // CUSTOMICON_H
