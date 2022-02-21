#include "editorstyle.h"
#include "util/custompaths.h"

#include <QDir>
#include <QJsonDocument>

namespace support_file {
namespace documents {
inline static QHash<QString, QJsonDocument> editorStyleGlobal;
inline static QHash<QString, QJsonDocument> editorStyleUser;
bool editorStyleIsLoaded(const QString &languageID);
} // namespace documents

bool documents::editorStyleIsLoaded(const QString &languageID)
{
    if (languageID.isEmpty())
        return false;

    return !editorStyleGlobal.value(languageID).isEmpty()
            && !editorStyleUser.value(languageID).isEmpty();
}

QString EditorStyle::globalPath(const QString &languageID)
{
    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Configures));
    return result + QString("editorstyle_%0.support").arg(languageID);
}

QString EditorStyle::userPath(const QString &languageID)
{
    auto result = CustomPaths::endSeparator(CustomPaths::user(CustomPaths::Configures));
    if (!CustomPaths::installed()) {
       result = CustomPaths::endSeparator(result + "temp");
       if (!QDir(result).exists()) {
           QDir().mkdir(result);
       }
    }
    return result + QString("editorstyle_%0.support").arg(languageID);
}

void EditorStyle::initialize(const QString &languageID)
{
    auto user = userPath(languageID);
    auto global = globalPath(languageID);

    if (!CustomPaths::installed()) {
        QFile::remove(user);
    }

    if (!QFileInfo(user).exists()) {
        QFile::copy(global, user);
    }
}

bool EditorStyle::recovery(const QString &languageID)
{
    bool result = false;
    auto user = userPath(languageID);
    result = QFile::remove(user);
    initialize(languageID);
    return result;
}

} // namespace support_file
