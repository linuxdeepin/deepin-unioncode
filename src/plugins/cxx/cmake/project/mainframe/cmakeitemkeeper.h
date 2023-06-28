// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMAKEITEMKEEPER_H
#define CMAKEITEMKEEPER_H

#include "common/common.h"

#include <QObject>
#include <QStandardItem>

class CmakeItemKeeperPrivate;
class CmakeItemKeeper : public Inotify
{
    Q_OBJECT
    Q_DISABLE_COPY(CmakeItemKeeper)
    CmakeItemKeeper();
    CmakeItemKeeperPrivate *const d;

public:
    static CmakeItemKeeper *instance();
    virtual ~CmakeItemKeeper();

signals:
    void cmakeFileNodeNotify(QStandardItem *rootItem, const QPair<QString, QStringList> &files);

public slots:
    void addCmakeRootFile(QStandardItem *root, const QString rootPath);
    void addCmakeSubFiles(QStandardItem *root, const QStringList subPaths);
    void delCmakeFileNode(QStandardItem *rootItem);

private:
    void notifyFromWatcher(const QString &filePath);
};

#endif // CMAKEITEMKEEPER_H
