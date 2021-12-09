/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#include "codetimecheck.h"

#include "framework/log/logutils.h"

//全局模块使能宏
#ifndef DPF_NO_CHECK_TIME

#include <QString>
#include <QFile>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>
#include <QMutex>
#include <QDateTime>
#include <QDate>
#include <QDir>
#include <QtConcurrent>

DPF_BEGIN_NAMESPACE

namespace GlobalPrivate {

    static QFile file;
    static uint dayCount = 7;
    static QMutex mutex;

#ifdef QT_NO_DEBUG
    const QString tcDirName = "codeTimeCheck";
    const QString tcFileName = "tc_release.csv";
#else
    const QString tcDirName = "codeTimeCheck";
    const QString tcFileName = "tc_debug.csv";
#endif

    static void rmExpiredLogs()
    {
        QtConcurrent::run([=](){
            QDirIterator itera(LogUtils::appCacheLogPath()
                               + "/" + tcDirName);
            while(itera.hasNext()) {
                itera.next();
                auto list = itera.fileName().split("_");
                if (itera.fileInfo().suffix() == "csv"
                        && list.count() == 3
                        && !LogUtils::containLastDay(
                            QDateTime(QDate::fromString(list[0],"yyyy-MM-dd"),
                                      QTime(0,0,0,0)),
                            LogUtils::toDayZero(),
                            GlobalPrivate::dayCount))
                {
                    qInfo("remove true(%d) not last week log: %s",
                          QDir().remove(itera.path() + "/" + itera.fileName()),
                          itera.fileName().toLocal8Bit().data());
                }
            }
        });
    }

    static void outCheck(const QMessageLogContext &context, const QString &msg)
    {
        //加锁保证内部函数执行时的互斥
        QMutexLocker lock(&GlobalPrivate::mutex);

        const QString &currAppLogName = LogUtils::appCacheLogPath()
                + "/" + tcDirName + "/"
                + LogUtils::localDate() + "_" + tcFileName;

        // "codeTimeCheck" dir
        LogUtils::checkAppCacheLogDir(tcDirName);

        // 文件名称为空 或者当前日期不正确(跨天日志分割)
        if (file.fileName().isEmpty()
                || file.fileName() != currAppLogName) {
            file.setFileName(currAppLogName);
            qInfo() << "Current checkTime file path: " << file.fileName();
            rmExpiredLogs();
        }

        bool isNewFile = false;
        if (!file.exists()) {
            isNewFile = true;
        }

        if (!file.isOpen()) {
            file.open(QFile::WriteOnly|QFile::Append);
        }

        if (isNewFile) {
            file.write((QString("时间") + ","
                        + "时间" + ","
                        + "毫秒" + ","
                        + "函数名称" + ","
                        + "动作类型" + ","
                        + "文件名称" + ","
                        + "文件行" + "\n").toUtf8().data());
            file.flush();
        }

        auto fileNameList = QString(context.file).split("/");
        auto currentName = fileNameList[fileNameList.size() - 1];

        file.write((LogUtils::localDataTimeCSV() + ","
                    + context.function + ","
                    + msg + ","
                    + currentName + ","
                    + QString::number(context.line)
                    + "\n").toUtf8().data());
        file.flush();
        file.close();
    }

}// namespace GlobalPrivate

/**
 * @brief setLogCacheDayCount 设置日志缓存时间
 *  需要在调用其他函数之前调用
 * @param dayCount 日志缓存时间
 */
void CodeCheckTime::setLogCacheDayCount(uint dayCount)
{
    GlobalPrivate::dayCount = dayCount;
}

/**
 * @brief logCacheDayCount 获取设置的日志缓存时间
 * @return uint 日志缓存时间，默认7天
 */
uint CodeCheckTime::logCacheDayCount()
{
    return GlobalPrivate::dayCount;
}

/**
 * @brief begin 检查点-开始
 * @param context 日志打印上下文，可参照QMessageLogContext
 */
void CodeCheckTime::begin(const QMessageLogContext &context)
{
    GlobalPrivate::outCheck(context, "begin");
}

/**
 * @brief end 检查点-结束
 * @param context 日志打印上下文，可参照QMessageLogContext
 */
void CodeCheckTime::end(const QMessageLogContext &context)
{
    GlobalPrivate::outCheck(context, "end");
}

#endif // DPF_NO_CHECK_TIME

DPF_END_NAMESPACE
