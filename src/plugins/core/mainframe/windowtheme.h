// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWTHEME_H
#define WINDOWTHEME_H

#include <QString>

class WindowTheme
{
public:
    WindowTheme() = delete;
    static void setTheme(const QString &file);
};

#endif // WINDOWTHEME_H
