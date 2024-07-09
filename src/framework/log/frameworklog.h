// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
class DPF_EXPORT FrameworkLog final
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
