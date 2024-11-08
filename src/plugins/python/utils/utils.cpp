// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

bool Utils::checkVenvValid(const QString &venvPath)
{
    QFileInfo info(venvPath);
    if (info.exists() && !info.isDir())
        return false;

    QDir binDir(venvPath + "/bin");
    if (!binDir.exists())
        return false;

    const auto &fileList = binDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
    auto iter = std::find_if(fileList.cbegin(), fileList.cend(),
                             [](const QString &file) {
                                 return file.startsWith("python");
                             });
    return iter != fileList.cend();
}

bool Utils::createVenv(const QString &python, const QString &venvPath)
{
    if (checkVenvValid(venvPath))
        return true;

    QProcess process;
    QStringList args { "-m", "venv", venvPath };
    process.setProgram(python);
    process.setArguments(args);
    process.start();
    process.waitForFinished();

    return process.exitCode() == 0;
}

QString Utils::pythonVersion(const QString &python)
{
    auto getVersion = [](const QString &output) -> QString {
        static QRegularExpression regex(R"((\d{1,3}(?:\.\d{1,3}){0,2}))");
        if (output.isEmpty())
            return "";

        QRegularExpressionMatch match = regex.match(output);
        if (match.hasMatch())
            return match.captured(1);

        return "";
    };

    QProcess process;
    process.start(python, { "--version" });
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    QString version = getVersion(output);
    if (version.isEmpty()) {
        output = process.readAllStandardError();
        version = getVersion(output);
        if (version.isEmpty()) {
            int index = python.lastIndexOf('/') + 1;
            output = python.mid(index, python.length() - index);
            version = getVersion(output);
        }
    }

    return version;
}

QString Utils::packageInstallPath(const QString &python)
{
    const auto &version = pythonVersion(python);
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
            + "/.unioncode/packages/Python" + version;
}
