// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LLASYNPARSE_H
#define LLASYNPARSE_H

#include "common/common.h"

class LLAsynParsePrivate;
class LLAsynParse : public QFileSystemWatcher
{
    Q_OBJECT
    LLAsynParsePrivate *const d;

public:
    template<class T>
    struct ParseInfo
    {
        T result;
        bool isNormal = true;
    };

    QSet<QString> getFilelist();

    LLAsynParse();
    virtual ~LLAsynParse();

signals:
    void itemsModified(const QList<QStandardItem *> &info);
    void parsedError(const ParseInfo<QString> &info);

public slots:
    void parseProject(const dpfservice::ProjectInfo &info);

private slots:
    void doDirectoryChanged(const QString &path);

private:
    void createRows(const QString &path);
    QString itemDisplayName(const QStandardItem *item) const;
    QStandardItem *findItem(const QString &path, QStandardItem *parent = nullptr) const;
    QList<QStandardItem *> rows(const QStandardItem *item) const;
    int separatorSize() const;
};

#endif   // LLASYNPARSE_H
