// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pipinstaller.h"
#include "utils/utils.h"

#include "common/util/eventdefinitions.h"
#include "services/option/optionmanager.h"
#include "services/terminal/terminalservice.h"
#include "services/window/windowelement.h"

#include <QProcess>
#include <QStandardPaths>

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
    return checkInstalled("python3", package);
}

void PIPInstaller::install(const InstallInfo &info)
{
    return install("python3", info);
}

bool PIPInstaller::checkPipExists(const QString &python)
{
    QProcess process;
    process.start(python, { "-m", "pip", "--version" });
    process.waitForFinished();

    return process.exitCode() == 0;
}

bool PIPInstaller::checkInstalled(const QString &python, const QString &package)
{
    // remove extra dependency
    // e.g: python-package[all] -> python-package
    QString pkgName = package;
    static QRegularExpression regex(R"(\[.*\])");
    if (pkgName.contains(regex))
        pkgName.remove(regex);

    QProcess process;
    auto env = process.processEnvironment();
    env.insert("PYTHONPATH", Utils::packageInstallPath(python));
    process.setProcessEnvironment(env);

    process.start(python, { "-m", "pip", "show", pkgName });
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    return !output.isEmpty();
}

void PIPInstaller::install(const QString &python, const InstallInfo &info)
{
    if (!termSrv)
        termSrv = dpfGetService(TerminalService);

    uiController.doSwitch(MWNA_EDIT);
    uiController.switchContext(TERMINAL_TAB_TEXT);
    const auto &map = OptionManager::getInstance()->getValue(option::CATEGORY_PYTHON, "Interpreter").toMap();
    const auto &pipSrc = map.value("pipSource").toString();

    // Check if pip exists, if not install python3-pip first
    if (!checkPipExists(python)) {
        // Install python3-pip and then install packages in one command
        QString installCommand = QString("apt install python3-pip -y && %1 -m pip install").arg(python);
        QStringList packages = info.packageList;
        installCommand += " " + packages.join(" ");
        installCommand += " --target " + Utils::packageInstallPath(python);
        if (!pipSrc.isEmpty())
            installCommand += " -i " + pipSrc;

        termSrv->executeCommand(info.plugin.isEmpty() ? "PIPInstaller" : info.plugin, "sudo", { installCommand }, "", QStringList());
    } else {
        // pip exists, install packages directly
        QStringList args { "-m", "pip", "install" };
        args << info.packageList
             << "--target"
             << Utils::packageInstallPath(python);

        if (!pipSrc.isEmpty())
            args << "-i" << pipSrc;

        termSrv->executeCommand(info.plugin.isEmpty() ? "PIPInstaller" : info.plugin, python, args, "", QStringList());
    }
}
