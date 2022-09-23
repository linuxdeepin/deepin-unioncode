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
#ifndef PROJECTPYTHONRECEIVER_H
#define PROJECTPYTHONRECEIVER_H

#include <framework/framework.h>

class ProjectPythonReceiver : public dpf::EventHandler,
        dpf::AutoEventHandlerRegister<ProjectPythonReceiver>
{
    Q_OBJECT
    friend class dpf::AutoEventHandlerRegister<ProjectPythonReceiver>;
public:
    explicit ProjectPythonReceiver(QObject * parent = nullptr);

    static Type type();

    static QStringList topics();

    virtual void eventProcess(const dpf::Event& event) override;
};

class ProjectPythonProxy : public QObject
{
    Q_OBJECT
    ProjectPythonProxy(){}
    ProjectPythonProxy(const ProjectPythonProxy&) = delete;
public:
    static ProjectPythonProxy* instance();
};

#endif // PROJECTPYTHONRECEIVER_H
