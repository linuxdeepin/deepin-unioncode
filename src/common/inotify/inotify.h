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
#ifndef INOTIFY_H
#define INOTIFY_H

#include <QObject>

class InotifyPrivate;
class Inotify final: public QObject
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
