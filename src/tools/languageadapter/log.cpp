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
#include "log.h"
#include "logutils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QtConcurrent>

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
    }

} // namespace GlobalPrivate

Log::Log()
{
    qInstallMessageHandler(&GlobalPrivate::redirectGlobalDebug);
}
