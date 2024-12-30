// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <QString>

class Utils
{
public:
    static bool checkVenvValid(const QString &venvPath);
    static bool createVenv(const QString &python, const QString &venvPath);
    static QString pythonVersion(const QString &python);
    static QString packageInstallPath(const QString &python);
    static QString defaultPIPSource();
};

#endif   // UTILS_H
