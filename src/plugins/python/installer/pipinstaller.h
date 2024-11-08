// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PIPINSTALLER_H
#define PIPINSTALLER_H

#include "base/abstractinstaller.h"

namespace dpfservice {
class TerminalService;
}

class PIPInstaller : public AbstractInstaller
{
    Q_OBJECT
public:
    explicit PIPInstaller(QObject *parent = nullptr);

    QString description() override;
    bool checkInstalled(const QString &package) override;
    void install(const InstallInfo &info) override;

    bool checkInstalled(const QString &python, const QString &package);
    void install(const QString &python, const InstallInfo &info);

private:
    dpfservice::TerminalService *termSrv { nullptr };
};

#endif   // PIPINSTALLER_H
