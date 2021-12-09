#ifndef FILESUFFIXDATABASE_H
#define FILESUFFIXDATABASE_H

#include <QObject>

class FileLangDatabasePrivate;
class FileLangDatabase final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileLangDatabase)
    FileLangDatabasePrivate *const d;
private:
    explicit FileLangDatabase(QObject *parent = nullptr);
    virtual ~FileLangDatabase();

public:
    static FileLangDatabase & instance();
    QString language(const QString &filePath);
};

#endif // FILESUFFIXDATABASE_H
