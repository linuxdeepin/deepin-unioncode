// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NINJAGENERATOR_H
#define NINJAGENERATOR_H

#include "services/language/languagegenerator.h"
#include "dap/protocol.h"

class NinjaGeneratorPrivate;
class NinjaGenerator : public dpfservice::LanguageGenerator
{
    Q_OBJECT
public:
    explicit NinjaGenerator();
    virtual ~NinjaGenerator() override;

    static QString toolKitName() { return "ninja"; }

    QString debugger() override;
    bool prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg) override;
    bool requestDAPPort(const QString &ppid, const QMap<QString, QVariant> &param, QString &retMsg) override;
    bool isNeedBuild() override;
    bool isTargetReady() override;
    bool isLaunchNotAttach() override;
    dap::LaunchRequest launchDAP(const QMap<QString, QVariant> &param) override;
    QString build(const QString &projectPath) override;
    QMap<QString, QVariant> getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                              const QString &currentFile) override;
    dpfservice::RunCommandInfo getRunArguments(const dpfservice::ProjectInfo &projectInfo,
                                               const QString &currentFile) override;

signals:

private slots:

private:
    NinjaGeneratorPrivate *const d;
};

#endif // NINJAGENERATOR_H
