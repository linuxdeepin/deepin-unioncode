// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
