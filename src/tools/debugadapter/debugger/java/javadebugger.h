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
#ifndef JAVADEBUGGER_H
#define JAVADEBUGGER_H


#include <QObject>

class JavaDebuggerPrivate;
class JavaDebugger : public QObject
{
    Q_OBJECT
public:
    explicit JavaDebugger(QObject *parent = nullptr);
    virtual ~JavaDebugger() override;

signals:
    void sigResolveClassPath(const QString &mainClass,
                             const QString &projectName);
    void sigCheckInfo();
    void sigSendToClient(const QString &uuid,
                         int port,
                         const QString &kit,
                         QMap<QString, QVariant> &param);

public slots:
    void slotReceivePojectInfo(const QString &uuid,
                               const QString &kit,
                               const QString &workspace,
                               const QString &configHomePath,
                               const QString &jrePath,
                               const QString &jreExecute,
                               const QString &launchPackageFile,
                               const QString &launchConfigPath,
                               const QString &dapPackageFile,
                               const QString &projectCachePath);
    void slotResolveClassPath(const QString &mainClass,
                              const QString &projectName);
    void slotCheckInfo();

private:
    void registerLaunchDAPConnect();

    void initialize(const QString &configHomePath,
                    const QString &jreExecute,
                    const QString &launchPackageFile,
                    const QString &launchConfigPath,
                    const QString &projectCachePath);
    void executeCommand(const QString &command);
    void parseResult(const QString &content);
    bool parseMainClass(const QString &content, QString &mainClass, QString &projectName);
    bool parseClassPath(const QString &content, QStringList &classPaths);

    JavaDebuggerPrivate *const d;
};

#endif // JAVADEBUGGER_H
