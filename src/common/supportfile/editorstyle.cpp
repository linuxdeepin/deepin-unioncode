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
#include "editorstyle.h"
#include "util/custompaths.h"

#include <QDir>
#include <QJsonDocument>

namespace support_file {
namespace documents {
inline static QHash<QString, QJsonDocument> editorStyleGlobal;
inline static QHash<QString, QJsonDocument> editorStyleUser;
bool editorStyleIsLoaded(const QString &languageID);
} // namespace documents

bool documents::editorStyleIsLoaded(const QString &languageID)
{
    if (languageID.isEmpty())
        return false;

    return !editorStyleGlobal.value(languageID).isEmpty()
            && !editorStyleUser.value(languageID).isEmpty();
}

QString EditorStyle::globalPath(const QString &languageID)
{
    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Configures));
    return result + QString("editorstyle_%0.support").arg(languageID);
}

QString EditorStyle::userPath(const QString &languageID)
{
    auto result = CustomPaths::endSeparator(CustomPaths::user(CustomPaths::Configures));
    if (!CustomPaths::installed()) {
       result = CustomPaths::endSeparator(result + "temp");
       if (!QDir(result).exists()) {
           QDir().mkdir(result);
       }
    }
    return result + QString("editorstyle_%0.support").arg(languageID);
}

void EditorStyle::initialize(const QString &languageID)
{
    auto user = userPath(languageID);
    auto global = globalPath(languageID);

    if (!CustomPaths::installed()) {
        QFile::remove(user);
    }

    if (!QFileInfo(user).exists()) {
        QFile::copy(global, user);
    }
}

bool EditorStyle::recovery(const QString &languageID)
{
    bool result = false;
    auto user = userPath(languageID);
    result = QFile::remove(user);
    initialize(languageID);
    return result;
}

} // namespace support_file
