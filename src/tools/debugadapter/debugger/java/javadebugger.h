/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer:
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
    void sigSendDAPPort(int port);

public slots:
    void slotReceivePojectInfo(QString workspace,
                               QString triggerFile,
                               QString configHomePath,
                               QString jrePath,
                               QString jreExecute,
                               QString launchPackageFile,
                               QString launchConfigPath,
                               QString dapPackageFile);

private:
    void registerLaunchDAPConnect();

    void initialize(QString configHomePath,
                    QString jreExecute,
                    QString launchPackageFile,
                    QString launchConfigPath);
    void executeCommand(QString command);
    void parseDAPPort(const QString &content);

    JavaDebuggerPrivate *const d;
};

#endif // JAVADEBUGGER_H
