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


