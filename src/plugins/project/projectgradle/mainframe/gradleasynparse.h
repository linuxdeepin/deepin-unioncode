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
#ifndef GRADLEASYNPARSE_H
#define GRADLEASYNPARSE_H

#include "services/project/projectinfo.h"

#include "common/common.h"

#include <QObject>
#include <QtXml>

class QStandardItem;
class QAction;
class GradleAsynParsePrivate;
class GradleAsynParse : public Inotify
{
    Q_OBJECT
    GradleAsynParsePrivate *const d;
public:
    template<class T>
    struct ParseInfo{
        T result;
        bool isNormal = true;
    };

    GradleAsynParse();
    virtual ~GradleAsynParse();

signals:
    void itemsModified(const dpfservice::ParseInfo<QList<QStandardItem*>> &info);
    void parsedError(const ParseInfo<QString> &info);

public slots:
    void loadPoms(const dpfservice::ProjectInfo &info);
    void parseProject(const dpfservice::ProjectInfo &info);

private slots:
    void doDirWatchModify(const QString &path);
    void doWatchCreatedSub(const QString &path);
    void doWatchDeletedSub(const QString &path);

private:
    bool isSame(QStandardItem *t1, QStandardItem *t2, Qt::ItemDataRole role = Qt::DisplayRole) const;
    void createRows(const QString &path);
    void removeRows();
    void removeSelfSubWatch(QStandardItem *item);
    QList<QStandardItem *> rows(const QStandardItem *item) const;
    int findRowWithDisplay(QList<QStandardItem*> rowList, const QString &fileName);
    QString itemDisplayName(const QStandardItem *item) const;
    QStandardItem *findItem(const QString &path, QStandardItem *parent = nullptr) const;
    int separatorSize() const;
    bool itemIsDir(const QStandardItem *item) const;
    bool itemIsFile(const QStandardItem *item) const;
    QStringList pathChildFileNames(const QString &path) const;
    QStringList displayNames(const QList<QStandardItem *> items) const;
    QStringList createdFileNames(const QString &path) const;
    QStringList deletedFileNames(const QString &path) const;
};

#endif // GRADLEASYNPARSE_H
