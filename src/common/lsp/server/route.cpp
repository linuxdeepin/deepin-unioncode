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
#include "route.h"

#include "serverapplication.h"

#include <QString>
#include <QDir>

namespace newlsp {

ProjectKey Route::key(QProcess *value) const
{
    return savedProcs.key(value);
}

QList<ProjectKey> Route::keys() const
{
    return savedProcs.keys();
}

QProcess *Route::value(const ProjectKey &key)
{
    QString language = QString::fromStdString(key.language);
    QProcess *ret{nullptr};
    ret = savedProcs.value(key);
    return ret;
}

void Route::save(const ProjectKey &key, QProcess *const value)
{
    if (!savedProcs.value(key)) {
        QObject::connect(value, QOverload<int>::of(&QProcess::finished),
                         this, &Route::deleteProc, Qt::DirectConnection);
        savedProcs.insert(key, value);
    }
}

void Route::deleteProc(int exitCode)
{
    Q_UNUSED(exitCode)
    QProcess *proc = qobject_cast<QProcess*>(sender());
    if (proc) {
        ProjectKey key = Route::key(proc);
        savedProcs.remove(key);
        proc->kill();
        proc->deleteLater();
    }
}

}
