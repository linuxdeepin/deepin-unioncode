// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EDITORSTYLE_H
#define EDITORSTYLE_H

#include "common/common_global.h"

#include <QString>

namespace support_file {

struct COMMON_EXPORT EditorStyle
{
    static QString globalPath(const QString &languageID);
    static QString userPath(const QString &languageID);
};

}

#endif // EDITORSTYLE_H
