#include "filelangdatabase.h"

#include <QDebug>
#include <QFileInfo>
#include <QJsonObject>

class FileLangDatabasePrivate
{
    friend class FileLangDatabase;
};

FileLangDatabase::FileLangDatabase(QObject *parent)
    : QObject(parent)
    , d(new FileLangDatabasePrivate)
{

}

FileLangDatabase::~FileLangDatabase()
{
    if (d)
        delete d;
}

FileLangDatabase &FileLangDatabase::instance()
{
    static FileLangDatabase db;
    return db;
}

QString FileLangDatabase::language(const QString &filePath)
{
    QFileInfo info(filePath);
    qInfo() << info.suffix();

    return "";
}


