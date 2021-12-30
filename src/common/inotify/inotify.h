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

    explicit Inotify(QObject *parent = nullptr);
    virtual ~Inotify();
    void subscribeTypes(Types types);
    bool addPath(const QString &path);
    void removePath(const QString &path);
    static Inotify *globalInstance();

signals:
    void modified(const QString &filePath); //File && Dir
    void opened(const QString &filePath); //File && Dir
    void closed(const QString &filePath); //File && Dir
    void movedSub(const QString &filePath); //Dir
    void createdSub(const QString &filePath); //Dir
    void deletedSub(const QString &filePath); //Dir
    void movedSelf(const QString &filePath); //File & &Dir
    void deletedSelf(const QString &filePath); //File && Dir
};

#endif // INOTIFY_H
