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
#ifndef MAVENASYNPARSE_H
#define MAVENASYNPARSE_H

#include "services/project/projectinfo.h"
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
    void parsedActions(const dpfservice::ProjectActionInfos &info);
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
