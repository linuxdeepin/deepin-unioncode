// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIRECTORYASYNPARSE_H
#define DIRECTORYASYNPARSE_H

#include "common/common.h"

#include <QObject>
#include <QSet>

class QStandardItem;
class QAction;
class DirectoryAsynParsePrivate;
class DirectoryAsynParse : public QFileSystemWatcher
{
    Q_OBJECT
    friend class DirectoryAsynParse;
    DirectoryAsynParsePrivate *const d;

public:
    template<class T>
    struct ParseInfo
    {
        T result;
        bool isNormal = true;
    };

    DirectoryAsynParse();
    QSet<QString> getFilelist();
    virtual ~DirectoryAsynParse();

signals:
    void itemsModified(const QList<QStandardItem *> &info);
    void parsedError(const ParseInfo<QString> &info);

public slots:
    void parseProject(const dpfservice::ProjectInfo &info);

private slots:
    void doDirectoryChanged(const QString &path);

private:
    void createRows(const QString &path);
    void sortAllRows();
    QString itemDisplayName(const QStandardItem *item) const;
    QStandardItem *findItem(const QString &path, QStandardItem *parent = nullptr) const;
    QList<QStandardItem *> rows(const QStandardItem *item) const;
    int separatorSize() const;
};

#endif   // DIRECTORYASYNPARSE_H
