// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
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
    static void projectActived(const dpfservice::ProjectInfo &info);
    static void projectCreated(const dpfservice::ProjectInfo &info);
    static void projectDeleted(const dpfservice::ProjectInfo &info);
    static void projectNodeExpanded(const QModelIndex &index);
    static void projectNodeCollapsed(const QModelIndex &index);
    static void collaboratorsOpenRepos(const QString &workspace);
};

#endif // SENDEVENTS_H
