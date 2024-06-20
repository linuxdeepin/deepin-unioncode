// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
