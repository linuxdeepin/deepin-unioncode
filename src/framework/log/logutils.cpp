// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logutils.h"

#include <QDateTime>
#include <QMutex>
#include <QDebug>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

DPF_BEGIN_NAMESPACE

namespace GlobalPrivate
{
    const QString cachePath = QStandardPaths::locate(QStandardPaths::CacheLocation,
                                                     "",
                                                     QStandardPaths::LocateDirectory);
    const QString deepinCachePath = cachePath + "deepin" + QDir::separator();
}

/**
 * @brief checkAppCacheLogDir
 *  检查应用程序缓存日志的文件夹
 * @param subDirName 日志目录下的子目录
 *  默认为空，则检查顶层目录
 */
void LogUtils::checkAppCacheLogDir(const QString &subDirName)
{
    if (!QFileInfo::exists(GlobalPrivate::deepinCachePath))
        QDir().mkdir(appCacheLogPath());

    if (!QFileInfo::exists(appCacheLogPath()))
        QDir().mkdir(appCacheLogPath());

    if (subDirName.isEmpty()) return;

    if (!QFileInfo::exists(appCacheLogPath() + QDir::separator() + subDirName))
        QDir().mkdir(appCacheLogPath() + QDir::separator() + subDirName);
}

/**
 * @brief appCacheLogPath
 *  获取当前应用程序的缓存日志路径，
 * @return QString 返回的结果总是一个Dir类型的字符路径
 */
QString LogUtils::appCacheLogPath()
{
    return GlobalPrivate::deepinCachePath + QCoreApplication::applicationName();
}

/**
 * @brief localDateTime 获取年/月/日/时间
 * @return QString 格式化字符串后的时间
 */
QString LogUtils::localDateTime()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

/**
 * @brief localDate 获取年/月/日
 * @return QString 格式化字符串后的时间
 */
QString LogUtils::localDate()
{
    return QDate::currentDate().toString("yyyy-MM-dd");
}

/**
 * @brief localDate 获取年/月/日/时间，
 *  年/月/日/与时间之间以逗号分割
 * @return QString 格式化字符串后的时间
 */
QString LogUtils::localDataTimeCSV()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd,hh:mm:ss,zzz");
}

/**
 * @brief lastTimeStamp 获取输入时间之前指定天数时间戳，
 * 最小单位s
 * @param dateTime 时间
 * @param dayCount 向前的天数
 * @return uint 时间戳
 */
uint LogUtils::lastTimeStamp(const QDateTime &dateTime, uint dayCount)
{
    return dateTime.toSecsSinceEpoch() - (86400 * dayCount);
}

/**
 * @brief lastDateTime 获取输入时间之前指定天数时间，
 * 最小单位s
 * @param dateTime 时间
 * @param dayCount 向前的天数
 * @return QDateTime 时间
 */
QDateTime LogUtils::lastDateTime(const QDateTime &dateTime, uint dayCount)
{
    return QDateTime::fromSecsSinceEpoch(lastTimeStamp(dateTime,dayCount));
}

/**
 * @brief containLastDay 判断时间是否包含时间(天)范围
 * 最小单位s
 * @param src 基准时间
 * @param dst 对比时间
 * @param dayCount 往前推的天数
 * @return bool 是否包含的结果
 *
 * |------dst--------src----|
 * |------dayCount----|
 * return true;
 *
 * |-----------dst-------------src|
 *                  |-dayCount--|
 * return false
 */
bool LogUtils::containLastDay(const QDateTime &src, const QDateTime &dst, uint dayCount)
{
    uint srcStamp = src.toSecsSinceEpoch();
    uint dstStamp = dst.toSecsSinceEpoch();

    return dstStamp - (86400 * dayCount) < srcStamp && srcStamp <= dstStamp;
}

/**
 * @brief toDayZero 获取今天的00:00:00的时间
 * @return
 */
QDateTime LogUtils::toDayZero()
{
    QDateTime dateTime;
    dateTime.setDate(QDate::currentDate());
    dateTime.setTime(QTime(0,0,0));
    return dateTime;
}

DPF_END_NAMESPACE
