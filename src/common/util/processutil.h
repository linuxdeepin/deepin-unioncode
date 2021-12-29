#ifndef PROCESSCALLER_H
#define PROCESSCALLER_H

#include <QString>
#include <QByteArray>
#include <functional>

class ProcessUtil final
{
    Q_DISABLE_COPY(ProcessUtil)
    ProcessUtil() = delete;
public:
    typedef std::function<void(const QByteArray &)> ReadCallBack;
    static bool execute(const QString &program,
                        const QStringList &arguments,
                        ReadCallBack func = nullptr);
    static bool exists(const QString &name);
    static QString version(const QString &name);

    static bool hasGio();
    static bool moveToTrash(const QString &filePath);
};
#endif // PROCESSCALLER_H
