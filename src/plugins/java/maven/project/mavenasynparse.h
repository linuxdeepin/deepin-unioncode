// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAVENASYNPARSE_H
#define MAVENASYNPARSE_H

#include "services/project/projectinfo.h"
#include "projectactioninfo.h"
#include "common/common.h"

#include <QFileSystemWatcher>
#include <QObject>
#include <QtXml>

class QStandardItem;
class QAction;
class MavenAsynParsePrivate;
class MavenAsynParse : public QFileSystemWatcher
{
    Q_OBJECT
    MavenAsynParsePrivate *const d;
public:
    MavenAsynParse();
    virtual ~MavenAsynParse();

signals:
    void parsedActions(const ProjectActionInfos &info);
    void parsedError(const QString &info);
    void itemsModified(const QList<QStandardItem*> &info);

public slots:
    void loadPoms(const dpfservice::ProjectInfo &info);
    void parseProject(const dpfservice::ProjectInfo &info);
    void parseActions(const dpfservice::ProjectInfo &info);

private slots:
    void doDirectoryChanged(const QString &path);

private:
    void createRows(const QString &path);
    QString itemDisplayName(const QStandardItem *item) const;
    QStandardItem *findItem(const QString &path, QStandardItem *parent = nullptr) const;
    QList<QStandardItem *> rows(const QStandardItem *item) const;
    int separatorSize() const;
};

#endif // MAVENASYNPARSE_H
