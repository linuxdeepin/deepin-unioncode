/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef LOGUTILS_H
#define LOGUTILS_H

#include "framework/framework_global.h"

#include <QString>
#include <QDateTime>

DPF_BEGIN_NAMESPACE

class LogUtils final
{
public:
    explicit LogUtils() = delete;
    static void checkAppCacheLogDir(const QString &subDirName = "");
    static QString appCacheLogPath();
    static QString localDateTime();
    static QString localDate();
    static QString localDataTimeCSV();
    static uint lastTimeStamp(const QDateTime &dateTime, uint dayCount);
    static QDateTime lastDateTime(const QDateTime &dateTime, uint dayCount);
    static bool containLastDay(const QDateTime &src, const QDateTime &dst, uint dayCount);
    static QDateTime toDayZero();
};

DPF_END_NAMESPACE

#endif // LOGUTILS_H
