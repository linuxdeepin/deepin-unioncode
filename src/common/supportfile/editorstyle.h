// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORSTYLE_H
#define EDITORSTYLE_H

#include <QString>

namespace support_file {

struct EditorStyle
{
    static QString globalPath(const QString &languageID);
    static QString userPath(const QString &languageID);
};

}

#endif // EDITORSTYLE_H
