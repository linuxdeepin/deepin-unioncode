/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
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
#include "configure.h"

#include "common/common.h"

QString Configure::filePath()
{
    return CustomPaths::user(CustomPaths::Flags::Configures)
            + QDir::separator() + "user_action_analyse.support";
}

QJsonDocument Configure::doc()
{
    QJsonDocument doc;
    QFile file(filePath());
    if (!file.exists()) {
        if (file.open(QFile::ReadWrite | QFile::NewOnly)) {
            QJsonObject defaultConf{ {"enabled", false} };
            doc.setObject(defaultConf);
            file.write(doc.toJson());
            file.close();
        }
    } else {
        if (file.open(QFile::ReadOnly)) {
            doc = QJsonDocument::fromJson(file.readAll());
            file.close();
        }
    }
    return doc;
}

bool Configure::enabled()
{
    return doc().object().value("enabled").toBool();
}

void Configure::setEnabled(bool enabled)
{
    QJsonObject obj = doc().object();
    obj["enabled"] = enabled;
    QJsonDocument doc(obj);
    QFile file(filePath());
    if (file.open(QFile::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
    actionanalyse.enabled(enabled);
}
