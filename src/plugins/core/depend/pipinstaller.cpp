// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pipinstaller.h"

#include "common/util/eventdefinitions.h"
#include "services/terminal/terminalservice.h"

#include <QProcess>

using namespace dpfservice;

PIPInstaller::PIPInstaller(QObject *parent)
    : AbstractInstaller(parent)
{
}

QString PIPInstaller::description()
{
    return tr("Install packages by pip");
}

bool PIPInstaller::checkInstalled(const QString &package)
{
    QProcess process;
    QString cmd("pip3 show %1");
    process.start(cmd.arg(package));
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    return !output.isEmpty();
}

void PIPInstaller::install(const InstallInfo &info)
{
    if (!termSrv)
        termSrv = dpfGetService(TerminalService);

    QStringList args;
    args << "install"
         << info.packageList;

    uiController.switchContext(tr("&Console"));
    termSrv->executeCommand(info.plugin.isEmpty() ? "PIPInstaller" : info.plugin, "pip3", args, "", QStringList());
}
