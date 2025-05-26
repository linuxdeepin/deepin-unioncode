// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "frameworklog.h"
#include "logutils.h"
#include "common/util/commandparser.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QtConcurrent>

#ifdef DTK_LOG
#include <DLog>
#endif

Q_LOGGING_CATEGORY(Framework, "Framework")

DPF_BEGIN_NAMESPACE

namespace GlobalPrivate
{
    static QFile file;
    static uint dayCount = 7;
    static QMutex mutex;

    QString formatFrameworkLogOut(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        auto fileNameList = QString(context.file).split(QDir::separator());
        auto currentName = fileNameList[fileNameList.size() - 1];

        if (type == QtMsgType::QtDebugMsg)
            return "[" + QString(context.category) + "]["
                    + LogUtils::localDateTime() + "][Debug]["
                    + currentName + " "
                    + context.function + " "
                    + QString::number(context.line) + "]"
                    + msg;
        if (type == QtMsgType::QtInfoMsg)
            return "[" + QString(context.category) + "]["
                    + LogUtils::localDateTime() + "][Info]["
                    + currentName + " "
                    + context.function + " "
                    + QString::number(context.line) + "]"
                    + msg;
        if (type == QtMsgType::QtCriticalMsg)
            return "[" + QString(context.category) + "]["
                    + LogUtils::localDateTime() + "][Critical]["
                    + currentName + " "
                    + context.function + " "
                    + QString::number(context.line) + "]"
                    + msg;
        if (type == QtMsgType::QtWarningMsg)
            return "[" + QString(context.category) + "]["
                    + LogUtils::localDateTime() + "][Warning]["
                    + currentName + " "
                    + context.function + " "
                    + QString::number(context.line) + "]"
                    + msg;
        if (type == QtMsgType::QtSystemMsg)
            return "[" + QString(context.category) + "]["
                    + LogUtils::localDateTime() + "][System]["
                    + currentName + " "
                    + context.function + " "
                    + QString::number(context.line) + "]"
                    + msg;
        if (type == QtMsgType::QtFatalMsg)
            return "[" + QString(context.category) + "]["
                    + LogUtils::localDateTime() + "][Fatal]["
                    + currentName + " "
                    + context.function + " "
                    + QString::number(context.line) + "]"
                    + msg;
        return msg;
    }

    static void rmExpiredLogs()
    {
        QtConcurrent::run([=](){
            QDirIterator itera(LogUtils::appCacheLogPath(),QDir::Files);
            while(itera.hasNext()) {
                itera.next();
                auto list = itera.fileName().split("_");
                if (itera.fileInfo().suffix() == "log"
                        && list.count() == 2
                        && !LogUtils::containLastDay(
                            QDateTime(QDate::fromString(list[0],"yyyy-MM-dd"),
                                      QTime(0,0,0,0)),
                            LogUtils::toDayZero(),
                            GlobalPrivate::dayCount))
                {

                    auto outMsg = formatFrameworkLogOut(QtMsgType::QtInfoMsg,
                                                        QMessageLogContext{
                                                            __FILE__,
                                                            __LINE__,
                                                            __FUNCTION__,
                                                            Framework().categoryName()
                                                        },
                                                        QString("remove true(%0) not last week log: %1")
                                                        .arg(QDir().remove(itera.path() + QDir::separator() + itera.fileName()))
                                                        .arg(itera.fileName().toLocal8Bit().data())
                                                        );

                    fprintf(stderr, "%s\n", outMsg.toUtf8().data());
                }
            }
        });
    }

    void redirectGlobalDebug(QtMsgType type,
                             const QMessageLogContext &context,
                             const QString &msg)
    {
        QMutexLocker locker(&mutex);
        QString logMsg = GlobalPrivate::formatFrameworkLogOut(type,context,msg);
        if (type == QtMsgType::QtDebugMsg)
            fprintf(stdin,"%s\n",logMsg.toUtf8().data());
        if (type == QtMsgType::QtInfoMsg)
            fprintf(stderr,"%s\n",logMsg.toUtf8().data());
        if (type == QtMsgType::QtSystemMsg)
            fprintf(stdin,"%s\n",logMsg.toUtf8().data());
        if (type == QtMsgType::QtCriticalMsg)
            fprintf(stderr,"%s\n",logMsg.toUtf8().data());
        if (type == QtMsgType::QtWarningMsg)
            fprintf(stderr,"%s\n",logMsg.toUtf8().data());
        if (type == QtMsgType::QtFatalMsg)
            fprintf(stderr,"%s\n",logMsg.toUtf8().data());

        // cache/deepin/qApp->applicationName()
        LogUtils::checkAppCacheLogDir();

        if (GlobalPrivate::file.fileName().isEmpty()) {
            GlobalPrivate::file.setFileName(LogUtils::appCacheLogPath()
                                            + QDir::separator()
                                            + LogUtils::localDate()
                                            + "_" + QCoreApplication::applicationName()
                                            + ".log");

            auto outMsg = GlobalPrivate::formatFrameworkLogOut(QtMsgType::QtInfoMsg,
                                                               QMessageLogContext{
                                                                   __FILE__,
                                                                   __LINE__,
                                                                   __FUNCTION__,
                                                                   Framework().categoryName()
                                                               },
                                                               "Current redirect log file path: "
                                                               + GlobalPrivate::file.fileName()
                                                               );

            fprintf(stderr, "%s\n", outMsg.toUtf8().data());

            //清除超出时间段的日志
            GlobalPrivate::rmExpiredLogs();
        }

        if (!GlobalPrivate::file.open(QFile::Append|QFile::ReadOnly)) {

            auto outMsg = GlobalPrivate::formatFrameworkLogOut(QtMsgType::QtInfoMsg,
                                                               QMessageLogContext{
                                                                   __FILE__,
                                                                   __LINE__,
                                                                   __FUNCTION__,
                                                                   Framework().categoryName()
                                                               },
                                                               "Failed, open redirect log file"
                                                               + GlobalPrivate::file.fileName()
                                                               + " "
                                                               + GlobalPrivate::file.errorString()
                                                               );

            fprintf(stderr, "%s\n", outMsg.toUtf8().data());

            return;
        }

        GlobalPrivate::file.write((logMsg + ("\n")).toLocal8Bit().data());
        GlobalPrivate::file.flush();
        GlobalPrivate::file.close();
    }

} // namespace GlobalPrivate

/**
 * @brief enableFrameworkLog 开启框架日志打印
 * @param enabled true为开启,false则关闭
 */
void FrameworkLog::enableFrameworkLog(bool enabled){
    if (enabled) {
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.warning=true"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.debug=true"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.info=true"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.critical=true"));

    } else {
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.warning=false"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.debug=false"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.info=false"));
        QLoggingCategory::setFilterRules(QLatin1String("FrameworkLog.critical=false"));
    }
}

/**
 * @brief setLogCacheDayCount 设置日志缓存时间，
 *  需要在调用其他函数之前调用
 * @param uint 缓存的天数
 */
void FrameworkLog::setLogCacheDayCount(uint dayCount)
{
    static QMutex mutex;
    mutex.lock();
    GlobalPrivate::dayCount = dayCount;
    mutex.unlock();
}

/**
 * @brief logCacheDayCount 获取设置的日志缓存时间
 * @return uint 缓存的天数,默认缓存7天
 */
uint FrameworkLog::logCacheDayCount()
{
    return GlobalPrivate::dayCount;
}

/**
 * @brief initialize 初始化框架日志打印模块
 */
void FrameworkLog::initialize()
{
#ifdef DTK_LOG

    QString tempPath = DTK_CORE_NAMESPACE::DLogManager::getlogFilePath();
    QString appName = QDir::separator() + qApp->applicationName() + QDir::separator();
    QString result = QDir::separator() + "deepin" + appName;
    DTK_CORE_NAMESPACE::DLogManager::setlogFilePath(tempPath.replace(appName,result));
    qInfo() << "redirect output info to log: " << DTK_CORE_NAMESPACE::DLogManager::getlogFilePath();
    DTK_CORE_NAMESPACE::DLogManager::registerConsoleAppender();
    DTK_CORE_NAMESPACE::DLogManager::registerFileAppender();
#else
    qInstallMessageHandler(&GlobalPrivate::redirectGlobalDebug);
#endif
}

DPF_END_NAMESPACE
