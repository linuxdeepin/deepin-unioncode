// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PYTHONASYNPARSE_H
#define PYTHONASYNPARSE_H

#include "common/common.h"

#include <QObject>
#include <QtXml>
#include <QSet>

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
    QSet<QString> getFilelist();
    virtual ~PythonAsynParse();

signals:
    void itemsModified(const QList<QStandardItem*> &info);
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

#endif // PYTHONASYNPARSE_H
