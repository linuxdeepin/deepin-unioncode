// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWCONTROLLER_H
#define WINDOWCONTROLLER_H

#include <QString>

// window Controller elements
namespace dpfservice {

enum Position {
    FullWindow = 0x0,
    Left = 0x1,
    Right = 0x2,
    Top = 0x3,
    Bottom = 0x4,
    Central = 0x5
};

// CM = Controller Mode
inline const QString CM_EDIT { "EDIT" };
inline const QString CM_DEBUG { "DEBUG" };
inline const QString CM_RECENT { "RECENT" };

}

#endif // WINDOWCONTROLLER_H
