// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JSAsynParse_H
#define JSAsynParse_H

#include "common/common.h"

#include <QObject>
#include <QtXml>

class QStandardItem;
class QAction;
class JSAsynParsePrivate;
class JSAsynParse : public QFileSystemWatcher
{
    Q_OBJECT
    friend class JSProjectGenerator;
    JSAsynParsePrivate *const d;
public:
    template<class T>
    struct ParseInfo{
        T result;
        bool isNormal = true;
    };

    JSAsynParse();
    virtual ~JSAsynParse();

signals:
    void itemsModified(const QList<QStandardItem*> &info);
    void parsedError(const ParseInfo<QString> &info);

public slots:
    void parseProject(dpfservice::ProjectInfo &info);

private slots:
    void doDirectoryChanged(const QString &path);

private:
    void createRows(const QString &path);
    QString itemDisplayName(const QStandardItem *item) const;
    QStandardItem *findItem(const QString &path, QStandardItem *parent = nullptr) const;
    QList<QStandardItem *> rows(const QStandardItem *item) const;
    int separatorSize() const;
    void iteratorDirectory(const QString &rootPath);
    void iteratorFiles(const QString &rootPath);
};

#endif // JSAsynParse_H
