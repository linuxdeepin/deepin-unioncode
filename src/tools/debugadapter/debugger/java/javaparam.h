/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#ifndef JAVAPARAM_H
#define JAVAPARAM_H


#include <QObject>

class JavaParam : public QObject
{
    Q_OBJECT
public:
    explicit JavaParam(QObject *parent = nullptr);
    virtual ~JavaParam() override;

    QString getInitBackendParam(const QString &port,
                                const QString &javaPath,
                                const QString &launcherPath,
                                const QString &heapDumpPath,
                                const QString &configLinuxPath,
                                const QString &dataPath);
    QString getLSPInitParam(const int requestId,
                            const int pid,
                            const QString &workspace,
                            const QString &jdkHome,
                            const QString &debugJar);
    QString getLSPInitilizedParam(const int requestId);
    QString getLaunchJavaDAPParam(const int requestId);
    QString getResolveMainClassParam(const int requestId,
                                     const QString &workspace);
    QString getResolveClassPathParam(const int requestId,
                                     const QString &mainClass,
                                     const QString &projectName);

signals:

public slots:
private:

};

#endif // JAVAPARAM_H
