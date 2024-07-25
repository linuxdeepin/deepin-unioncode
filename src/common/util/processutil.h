// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

    static bool execute(const QString &program,
                        const QStringList &arguments,
                        const QString &workdir,
                        const QProcessEnvironment &env,
                        ReadCallBack func = nullptr);

    static QString execute(const QStringList &commands, bool cascade);

    static bool exists(const QString &name);
    static QString version(const QString &name);

    static bool hasGio();
    static bool moveToTrash(const QString &filePath);
    static bool recoverFromTrash(const QString &filePath);
    static bool portOverhead(unsigned int port);
    static QString localPlatform();
};
#endif // PROCESSCALLER_H
