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
#ifndef PROJECTRECEIVER_H
#define PROJECTRECEIVER_H

#include "services/project/projectinfo.h"
#include "services/builder/builderglobals.h"

#include <framework/framework.h>

class ProjectCmakeReceiver : public dpf::EventHandler,
        dpf::AutoEventHandlerRegister<ProjectCmakeReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<ProjectCmakeReceiver>;
public:
    explicit ProjectCmakeReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;

    virtual void builderEvent(const dpf::Event& event);

    virtual void recentEvent(const dpf::Event& event);
};

class ProjectCmakeProxy : public QObject
{
    Q_OBJECT
    ProjectCmakeProxy(){}
    ProjectCmakeProxy(const ProjectCmakeProxy&) = delete;

public:
    static ProjectCmakeProxy* instance();

signals:
    void buildExecuteEnd(const BuildCommandInfo &commandInfo, int status = 0);
};

#endif // PROJECTRECEIVER_H
