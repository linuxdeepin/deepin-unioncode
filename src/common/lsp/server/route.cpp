// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
        QObject::connect(value, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                         this, &Route::deleteProc, Qt::DirectConnection);
        savedProcs.insert(key, value);
    }
}

void Route::deleteProc(int exitCode, QProcess::ExitStatus exitstatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitstatus)
    QProcess *proc = qobject_cast<QProcess*>(sender());
    if (proc) {
        ProjectKey key = Route::key(proc);
        savedProcs.remove(key);
        proc->kill();
        proc->deleteLater();
    }
}

}
