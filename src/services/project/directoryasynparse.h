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
    DirectoryAsynParsePrivate *const d;

public:
    template<class T>
    struct ParseInfo
    {
        T result;
        bool isNormal = true;
    };

    explicit DirectoryAsynParse();
    virtual ~DirectoryAsynParse();

    QSet<QString> getFilelist();
    QStandardItem *findItem(const QString &path, QStandardItem *parent = nullptr) const;
    void updateItem(QStandardItem *item);

signals:
    void itemsCreated(QList<QStandardItem *> itemList);
    void reqUpdateItem(const QString &path);
    void parsedError(const ParseInfo<QString> &info);

public slots:
    void parseProject(const dpfservice::ProjectInfo &info);

private slots:
    void doDirectoryChanged(const QString &path);

private:
    void createRows(const QString &path);
    QStandardItem *findParentItem(const QString &path, QStandardItem *parent = nullptr) const;
    QList<QStandardItem *> rows(const QStandardItem *item) const;
    QList<QStandardItem *> takeAll(QStandardItem *item);

    void sortItems();
    void sortChildren(QStandardItem *parentItem);
    bool compareItems(QStandardItem *item1, QStandardItem *item2);
};

#endif   // DIRECTORYASYNPARSE_H
