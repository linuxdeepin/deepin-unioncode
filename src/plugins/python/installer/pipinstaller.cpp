// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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

    QStringList args { "-m", "pip", "install" };
    args << info.packageList
         << "--target"
         << Utils::packageInstallPath(python);

    const auto &map = OptionManager::getInstance()->getValue(option::CATEGORY_PYTHON, "Interpreter").toMap();
    const auto &pipSrc = map.value("pipSource").toString();
    if (!pipSrc.isEmpty())
        args << "-i" << pipSrc;

    uiController.switchContext(TERMINAL_TAB_TEXT);
    termSrv->executeCommand(info.plugin.isEmpty() ? "PIPInstaller" : info.plugin, python, args, "", QStringList());
}
