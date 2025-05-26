// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INOTIFY_HOOK_H
#define INOTIFY_HOOK_H

#include <QThread>
#include <QHash>
#include <QReadWriteLock>

class InotifyHook : public QThread
{
    Q_OBJECT
public:
    enum Type
    {
        ACCESS,         // 文件被访问
        MODIFY,         // 文件被修改
        ATTRIB,         // 文件属性被修改
        CLOSE_WRITE,    // 以写打开的文件被关闭
        CLOSE_NOWRITE,  // 已不可写打开的文件被关闭
        CLOSE,          // 文件关闭
        OPEN,           // 文件打开
        MOVED_FROM,     // 文件被移出监控目录
        MOVED_TO,       // 文件移入监控目录
        MOVE,           // 文件被移动
        CREATE,         // 新建文件或文件夹
        DELETE,         // 文件或目录被删除
        DELETE_SELF,    // 监控目录被删除
        MOVE_SELF,      // 监控目录被移动
    };
    Q_ENUM(Type)
    Q_DECLARE_FLAGS(Types, Type)
    Q_FLAG(Types)

    virtual void stop() { stopFlag = true; }

    virtual void addPath(const QString &path)
    { Q_UNUSED(path); }

    virtual void removePath(const QString &path)
    { Q_UNUSED(path); }

signals:
    void inotifyEvent(Type type, const QString &filePath);

protected:
    bool stopFlag = false;
};

#endif // INOTIFY_HOOK_H
