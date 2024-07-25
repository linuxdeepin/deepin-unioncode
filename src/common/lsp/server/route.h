// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROUTE_H
#define ROUTE_H

#include "common/lsp/protocol/newprotocol.h"
#include "common/common_global.h"

#include <QObject>
#include <QProcess>

namespace newlsp {
class COMMON_EXPORT Route : public QObject
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
