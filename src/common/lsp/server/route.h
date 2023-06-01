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
#ifndef ROUTE_H
#define ROUTE_H

#include "common/lsp/protocol/newprotocol.h"

#include <QObject>
#include <QProcess>

namespace newlsp {
class Route : public QObject
{
    Q_OBJECT
public:
    explicit Route(QObject *parent = nullptr) : QObject(parent){}
    ProjectKey key(QProcess *value) const;
    QList<ProjectKey> keys() const;
    QProcess *value(const ProjectKey &key);
    void save(const ProjectKey &key, QProcess *const value);

private Q_SLOTS:
    void deleteProc(int exitCode, QProcess::ExitStatus exitstatus);

private:
    Route(){}
    QHash<ProjectKey, QProcess*> savedProcs;
};

}
#endif // ROUTE_H
