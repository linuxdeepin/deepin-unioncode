// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "common/type/menuext.h"

#include <QString>
#include <QSet>

namespace support_file {

struct Language
{
    enum_def(Key_2, QString)
    {
        enum_exp suffix = "suffix";
        enum_exp base = "base";
        enum_exp mimeType = "mimeType";
        enum_exp tokenWords = "tokenWords";
    };

    static QString globalPath();
    static QString userPath();

    static void initialize();
    static bool recovery();

    static QStringList ids();
    static QString id(const QString &filePath);
    static QString idAlias(const QString &id);
    static QMap<int, QString> tokenWords(const QString &id);
    static QSet<QString> suffixs(const QString &id);
    static QSet<QString> bases(const QString &id);
    static QSet<QString> mimeTypes(const QString &id);
};

}

#endif // LANGUAGE_H
