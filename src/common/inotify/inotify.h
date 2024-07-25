// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INOTIFY_H
#define INOTIFY_H

#include "common/common_global.h"

#include <QObject>

class InotifyPrivate;
class COMMON_EXPORT Inotify : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Inotify)
    InotifyPrivate * const d;
public:
    explicit Inotify(QObject *parent = nullptr);
    virtual ~Inotify();
    static Inotify *globalInstance();

public slots:
    void addPath(const QString &path);
    void removePath(const QString &path);
    void addIgnorePath(const QString &path);
    void removeIgnorePath(const QString &path);

signals:
    void modified(const QString &filePath); //File && Dir
    void opened(const QString &filePath); //File && Dir
    void closed(const QString &filePath); //File && Dir
    void movedSub(const QString &filePath); //Dir
    void createdSub(const QString &filePath); //Dir
    void deletedSub(const QString &filePath); //Dir
    void movedSelf(const QString &filePath); //File & &Dir
    void deletedSelf(const QString &filePath); //File && Dir

    void ignoreModified(const QString &filePath); //File && Dir
    void ignoreOpened(const QString &filePath); //File && Dir
    void ignoreClosed(const QString &filePath); //File && Dir
    void ignoreMovedSub(const QString &filePath); //Dir
    void ignoreCreatedSub(const QString &filePath); //Dir
    void ignoreDeletedSub(const QString &filePath); //Dir
    void ignoreMovedSelf(const QString &filePath); //File & &Dir
    void ignoreDeletedSelf(const QString &filePath); //File && Dir
};

#endif // INOTIFY_H
