// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTIL_H
#define UTIL_H

#include <QString>

class Util
{
public:
    static bool checkPackageValid(const QString &package);
    static void installPackage(const QString &package);
};

#endif // UTIL_H
