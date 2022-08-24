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
#ifndef PROCESSCALLER_H
#define PROCESSCALLER_H

#include <QString>
#include <QProcess>
#include <QByteArray>

#include <functional>

class ProcessUtil final
{
    Q_DISABLE_COPY(ProcessUtil)
    ProcessUtil() = delete;
public:
    typedef std::function<void(int, QProcess::ExitStatus)> FinishedCallBack;
    typedef std::function<void(const QByteArray &)> ReadCallBack;
    static bool execute(const QString &program,
                        const QStringList &arguments,
                        ReadCallBack func = nullptr);

    static bool execute(const QString &program,
                        const QStringList &arguments,
                        const QString &workdir,
                        ReadCallBack func = nullptr);

    static bool exists(const QString &name);
    static QString version(const QString &name);

    static bool hasGio();
    static bool moveToTrash(const QString &filePath);
    static bool recoverFromTrash(const QString &filePath);
};
#endif // PROCESSCALLER_H
