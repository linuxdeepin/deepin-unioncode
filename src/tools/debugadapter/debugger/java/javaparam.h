// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
