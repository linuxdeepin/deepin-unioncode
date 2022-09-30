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

#include <framework/framework.h>

class ProjectGradleReceiver : public dpf::EventHandler,
        dpf::AutoEventHandlerRegister<ProjectGradleReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<ProjectGradleReceiver>;
public:
    explicit ProjectGradleReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

class ProjectGradleProxy : public QObject
{
    Q_OBJECT
    ProjectGradleProxy(){}
    ProjectGradleProxy(const ProjectGradleProxy&) = delete;
public:
    static ProjectGradleProxy* instance();
};

#endif // PROJECTRECEIVER_H
