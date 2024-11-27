// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolchain.h"
#include "util/processutil.h"
#include "util/custompaths.h"

#include <QFile>
#include <QDir>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

namespace toolchains {
const QString K_SCIRPTNAME { "toolchain/main.py" };
const QString K_TOOLCHAINFILE { "toolchains.json" };
const QString K_VERSION { "version" };
const QString K_HOSTK_TOOLCHAINFILEK_TOOLCHAINFILEK_TOOLCHAINFILE_OS { "host_os" };
const QString K_HOST_ARCH { "host_arch" };
const QString K_HOST_KERNEL { "host_kernel" };
const QString K_TOOLCHAINS { "toolchains" };
const QString K_TOOLCHAIN_NAME { "toolchain_name" };
const QString K_TOOLCHAIN_ABI { "toolchain_abi" };
const QString K_TOOLCHAIN_PREFIX { "toolchain_prefix" };
const QString K_TOOLCHAIN_PATH { "toolchain_path" };
const QString K_TOOLCHAIN_C_COMPILER { "toolchain_c_compiler" };
const QString K_TOOLCHAIN_CXX_COMPILER { "toolchain_cxx_compiler" };
const QString K_TOOLCHAIN_DEBUGGER { "toolchain_debugger" };
}   // namespace toolchain

bool toolchains::generatGlobalFile()
{
    auto script = CustomPaths::global(CustomPaths::Scripts) + QDir::separator() + toolchains::K_SCIRPTNAME;
    if (!QFileInfo(script).isFile())
        return false;

    QString outputFile = CustomPaths::user(CustomPaths::Configures) + QDir::separator() + toolchains::K_TOOLCHAINFILE;
    QStringList args { script, "-o", outputFile };
    ProcessUtil::execute("python3", args, [=](const QByteArray &out) {
        qInfo() << out;
    });

    if (QFile(outputFile).exists())
        return true;

    return false;
}
