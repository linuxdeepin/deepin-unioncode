#include "stylejsonfile.h"
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
    QHash<QString, QJsonObject> userObjects{};
    QString languageID;
    QJsonObject themeObj;
    bool load(const QString &languageID);
};

StyleJsonFile::StyleJsonFile()
    : d(new StyleJsonFilePrivate())
{

}

StyleJsonFile::~StyleJsonFile()
{
     if (d) {
        delete d;
     }
}

bool StyleJsonFile::setLanguage(const QString &languageID)
{
    QMutexLocker locker(&::mutex);

    bool result = false;
    result &= d->load(languageID);
    if (result) {
        d->languageID = languageID;
    }
    return result;
}

QStringList StyleJsonFile::themes()
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

    SupportFile::EditorStyle::initialize(languageID);

    if (!userObjects[languageID].isEmpty())
        return true;

    QString languageStyleFilePath = SupportFile::EditorStyle::userPath(languageID);
    QFile file(languageStyleFilePath);

    if (!file.exists()) {
        return false;
    }

    QJsonObject obj;
    if (file.open(QFile::ReadOnly)){
        obj = QJsonDocument::fromJson(file.readAll()).object();
        userObjects[languageID] = obj;
        file.close();
    }
    return true;
}
