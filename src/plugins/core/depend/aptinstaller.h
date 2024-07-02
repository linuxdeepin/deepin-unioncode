// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APTINSTALLER_H
#define APTINSTALLER_H

#include "base/abstractinstaller.h"

namespace dpfservice {
class TerminalService;
}

class APTInstaller : public AbstractInstaller
{
    Q_OBJECT
public:
    explicit APTInstaller(QObject *parent = nullptr);

    QString description() override;
    bool checkInstalled(const QString &package) override;
    void install(const InstallInfo &info) override;

private:
    dpfservice::TerminalService *termSrv { nullptr };
};

#endif   // APTINSTALLER_H
