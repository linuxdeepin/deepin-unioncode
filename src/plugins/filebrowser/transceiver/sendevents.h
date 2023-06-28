// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SENDEVENTS_H
#define SENDEVENTS_H

#include <QString>
#include "services/project/projectservice.h"

class SendEvents final
{
    SendEvents() = delete;
    SendEvents(const SendEvents &) = delete;
public:
    static void buildProject(const QString &buildSystem,
                             const QString &buildDir,
                             const QString &buildFilePath,
                             const QStringList &buildArgs);

    static void gengrateWorkspace(const QString &projectPath);
};

#endif // SENDEVENTS_H
