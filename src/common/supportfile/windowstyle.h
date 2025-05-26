// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWSTYLE_H
#define WINDOWSTYLE_H

#include <QString>
namespace support_file {

struct WindowStyle
{
    static QString globalPath();
    static QString userPath();
};

}
#endif // WINDOWSTYLE_H
