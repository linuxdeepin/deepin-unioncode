// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
                    const QString &workspace);
    void executeCommand(const QString &command);
    void parseResult(const QString &content);
    bool parseMainClass(const QString &content, QString &mainClass, QString &projectName);
    bool parseClassPath(const QString &content, QStringList &classPaths);
    void outputMsg(const QString &title, const QString &msg);

    JavaDebuggerPrivate *const d;
};

#endif // JAVADEBUGGER_H
