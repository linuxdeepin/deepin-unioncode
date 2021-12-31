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
#ifndef FRAMEWORKLOG_H
#define FRAMEWORKLOG_H

#include "framework/log/codetimecheck.h"

#include "framework/framework_global.h"

#include <QDebug>
#include <QLoggingCategory>

#ifdef DTK_LOG
#define dpfDebug() qDebug() << "Framework:"
#define dpfInfo() qInfo() << "Framework:"
#define dpfWarning() qWarning() << "Framework:"
#define dpfCritical() qCritical() << "Framework:"
#else
/**
 * @brief Framework 可进行相关调用，如下
 * @code
 * qCDebug(Framework) << "hello";    //调试信息打印
 * qCInfo(Framework) << "hello";    //信息打印
 * qCDWarning(Framework) << "hello";     //警告打印
 * qCCritical(Framework) << "hello";     //关键日志打印
 * @endcode
 */
Q_DECLARE_LOGGING_CATEGORY(Framework)

/**
 * @brief FrameworkLog 宏函数，可进行相关打印调用，如下
 * @code
 * dpfDebug() << "hello";    //调试信息打印
 * dpfInfo() << "hello";    //信息打印
 * dpfWarning() << "hello";     //警告打印
 * dpfCritical() << "hello";     //关键日志打印
 * @endcode
 */
#define dpfDebug() qCDebug(Framework)
#define dpfInfo() qCInfo(Framework)
#define dpfWarning() qCDWarning(Framework)
#define dpfCritical() qCCritical(Framework)
#endif

DPF_BEGIN_NAMESPACE

/**
 * @brief The FrameworkLog class
 *  框架日志打印模块，内部封装输出重定向与日志格式化
 */
class FrameworkLog final
{
public:
    explicit FrameworkLog() = delete;
    static void enableFrameworkLog(bool enabled = true);
    static void setLogCacheDayCount(uint dayCount);
    static uint logCacheDayCount();
    static void initialize();
};

DPF_END_NAMESPACE

#endif // FRAMEWORKLOG_H
