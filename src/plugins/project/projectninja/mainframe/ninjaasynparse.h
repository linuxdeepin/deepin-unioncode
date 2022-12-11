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
#ifndef NINJAASYNPARSE_H
#define NINJAASYNPARSE_H

#include "services/project/projectinfo.h"

#include "common/common.h"

#include <QFileSystemWatcher>
#include <QObject>
#include <QtXml>

class QStandardItem;
class QAction;
class NinjaAsynParsePrivate;
class NinjaAsynParse : public QFileSystemWatcher
{
    Q_OBJECT
    NinjaAsynParsePrivate *const d;
public:
    template<class T>
    struct ParseInfo{
        T result;
        bool isNormal = true;
    };

    NinjaAsynParse();
    virtual ~NinjaAsynParse();

signals:
    void itemsModified(const QList<QStandardItem*> &info);
    void parsedError(const ParseInfo<QString> &info);

public slots:
    void parseProject(const dpfservice::ProjectInfo &info);

private slots:
    void doDirectoryChanged(const QString &path);

private:
    void createRows(const QString &path);
    QList<QStandardItem *> rows(const QStandardItem *item) const;
    QString itemDisplayName(const QStandardItem *item) const;
    QStandardItem *findItem(const QString &path, QStandardItem *parent = nullptr) const;
    int separatorSize() const;
};

#endif // NINJAASYNPARSE_H
