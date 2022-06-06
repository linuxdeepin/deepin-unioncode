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
#include "stylejsonfile.h"
#include "textedittabwidget/textedit.h"
#include "common/common.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMutex>

namespace {
static QMutex mutex;
}

class StyleJsonFilePrivate
{
    friend class StyleJsonFile;
    TextEdit *edit;
    QHash<QString, QJsonObject> userObjects{};
    QString languageID;
    QJsonObject themeObj;
    bool load(const QString &languageID);
};

StyleJsonFile::StyleJsonFile(TextEdit *edit)
    : QObject (edit)
    , d(new StyleJsonFilePrivate())
{
    d->edit = edit;
}

TextEdit *StyleJsonFile::edit()
{
    return d->edit;
}

StyleJsonFile::~StyleJsonFile()
{
    if (d) {
        delete d;
    }
}

bool StyleJsonFile::setLanguage(const QString &languageID)
{
    bool result = false;
    result = d->load(languageID);

    QMutexLocker locker(&::mutex);
    if (result) {
        d->languageID = languageID;
    }
    return result;
}

QStringList StyleJsonFile::themes() const
{
    QMutexLocker locker(&::mutex);

    if (d->languageID.isEmpty())
        return {};

    return d->userObjects[d->languageID].keys();
}

bool StyleJsonFile::setTheme(const QString &theme)
{
    QMutexLocker locker(&::mutex);

    if (d->languageID.isEmpty()) {
        qCritical() << "Failed, Language is no setting, "
                       "call setTheme after need to call setLanguage";
        return false;
    }

    QJsonObject fileObj= d->userObjects.value(d->languageID);
    if (!fileObj.keys().contains(theme))
        return false;

    d->themeObj = fileObj.value(theme).toObject();

    if (d->themeObj.isEmpty())
        return false;

    return true;
}

QJsonValue StyleJsonFile::value(const QString &Key) const
{
    QMutexLocker locker(&::mutex);
    return d->themeObj.value(Key);
}

bool StyleJsonFilePrivate::load(const QString &languageID)
{
    QMutexLocker locker(&::mutex);

    support_file::EditorStyle::initialize(languageID);

    if (!userObjects[languageID].isEmpty())
        return true;

    QString languageStyleFilePath = support_file::EditorStyle::userPath(languageID);
    QFile file(languageStyleFilePath);

    if (!file.exists()) {
        return false;
    }

    QJsonObject obj;
    if (file.open(QFile::ReadOnly)){
        QJsonParseError error;
        auto jsonDoc = QJsonDocument::fromJson(file.readAll(), &error);
        if (error.error != QJsonParseError::NoError) {
            qCritical() << error.errorString();
        }
        obj = jsonDoc.object();
        userObjects[languageID] = obj;
        file.close();
    }
    return true;
}
