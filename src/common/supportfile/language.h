/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
        enum_exp server = "server";
        enum_exp serverArguments = "serverArguments";
        enum_exp tokenWords = "tokenWords";
    };

    struct ServerInfo
    {
        QString progrma;
        QStringList arguments;
    };

    static QString globalPath();
    static QString userPath();

    static void initialize();
    static bool recovery();

    static QStringList ids();
    static QString id(const QString &filePath);
    static ServerInfo sever(const QString &id);
    static QMap<int, QString> tokenWords(const QString &id);
    static QSet<QString> suffixs(const QString &id);
    static QSet<QString> bases(const QString &id);
    static QSet<QString> mimeTypes(const QString &id);
};

}

#endif // LANGUAGE_H
