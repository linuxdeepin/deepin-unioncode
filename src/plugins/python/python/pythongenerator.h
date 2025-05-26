// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PYTHONGENERATOR_H
#define PYTHONGENERATOR_H

#include "services/language/languagegenerator.h"
#include "dap/protocol.h"

class PythonGeneratorPrivate;
class PythonGenerator : public dpfservice::LanguageGenerator
{
    Q_OBJECT
public:
    explicit PythonGenerator();
    virtual ~PythonGenerator() override;

    static QString toolKitName() { return "python"; }
    QString debugger() override;

    bool isTargetReady() override;
    bool prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg) override;
    bool requestDAPPort(const QString &ppid, const QMap<QString, QVariant> &param, QString &retMsg) override;
    bool isLaunchNotAttach() override;
    dap::AttachRequest attachDAP(int port, const QMap<QString, QVariant> &param) override;
    bool isRestartDAPManually() override;
    bool isStopDAPManually() override;
    QMap<QString, QVariant> getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                              const QString &currentFile) override;
    dpfservice::RunCommandInfo getRunArguments(const dpfservice::ProjectInfo &projectInfo,
                                                             const QString &currentFile) override;
signals:

private slots:

private:
    PythonGeneratorPrivate *const d;
};

#endif // PYTHONGENERATOR_H
