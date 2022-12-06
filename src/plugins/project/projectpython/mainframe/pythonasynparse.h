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
#ifndef PYTHONASYNPARSE_H
#define PYTHONASYNPARSE_H

#include "services/project/projectinfo.h"

#include "common/common.h"

#include <QObject>
#include <QtXml>

class QStandardItem;
class QAction;
class PythonAsynParsePrivate;
class PythonAsynParse : public QFileSystemWatcher
{
    Q_OBJECT
    friend class PythonGenerator;
    PythonAsynParsePrivate *const d;
public:
    template<class T>
    struct ParseInfo{
        T result;
        bool isNormal = true;
    };

    PythonAsynParse();
    virtual ~PythonAsynParse();

signals:
    void itemsModified(const QList<QStandardItem*> &info);
    void parsedError(const ParseInfo<QString> &info);

public slots:
    void loadPoms(const dpfservice::ProjectInfo &info);
    void parseProject(const dpfservice::ProjectInfo &info);

private slots:
    void doDirWatchModify(const QString &path);
    void doWatchCreatedSub(const QString &path);
    void doDirectoryChanged(const QString &path);

private:
    void createRows(const QString &path);
    QString itemDisplayName(const QStandardItem *item) const;
    QStandardItem *findItem(const QString &path, QStandardItem *parent = nullptr) const;
    QList<QStandardItem *> rows(const QStandardItem *item) const;
    int separatorSize() const;
};

#endif // PYTHONASYNPARSE_H
