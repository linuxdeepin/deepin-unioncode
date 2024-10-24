// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pipinstaller.h"

#include "common/util/eventdefinitions.h"
#include "services/terminal/terminalservice.h"
#include "services/window/windowelement.h"

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
    // remove extra dependency
    // e.g: python-package[all] -> python-package
    QString pkgName = package;
    QRegularExpression regex(R"(\[.*\])");
    if (pkgName.contains(regex))
        pkgName.remove(regex);

    QProcess process;
    QString cmd("pip3 show %1");
    process.start(cmd.arg(pkgName));
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

    uiController.switchContext(TERMINAL_TAB_TEXT);
    termSrv->executeCommand(info.plugin.isEmpty() ? "PIPInstaller" : info.plugin, "pip3", args, "", QStringList());
}
