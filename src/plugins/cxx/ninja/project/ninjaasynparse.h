// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NINJAASYNPARSE_H
#define NINJAASYNPARSE_H

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
