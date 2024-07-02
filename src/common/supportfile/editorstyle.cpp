// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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
    QDir dir(result);
    if (!dir.exists()) {
        QStringList list = result.split(QDir::separator());
        list.removeFirst();
        QDir dir(QDir::separator());
        for (auto &val : list) {
            if (dir.cd(val)) {
                continue;
            } else {
                dir.mkdir(val);
            }
        }
        if (!CustomPaths::installed()) {
            auto tempConfigFile = CustomPaths::endSeparator(result + "temp");
            QDir dir(tempConfigFile);
            if (!dir.exists())
                QDir(result).mkdir("temp");
            result = tempConfigFile;
        }
    }
    return result + QString("editorstyle_%0.support").arg(languageID);
}

} // namespace support_file
