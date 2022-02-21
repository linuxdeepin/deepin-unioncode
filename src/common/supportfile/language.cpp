#include "language.h"
#include "util/custompaths.h"
#include "dialog/contextdialog.h"

#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>

namespace support_file {

namespace documents {
inline static QJsonDocument languageGlobal;
inline static QJsonDocument languageUser;
bool languageIsLoaded();
} //namespace documents

bool documents::languageIsLoaded()
{
    return !languageGlobal.isEmpty() && !languageUser.isEmpty();
}

QString Language::globalPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Configures));
    return result + "language.support";
}

QString Language::userPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::user(CustomPaths::Configures));
    return result + "language.support";
}

void Language::initialize()
{
    if (!documents::languageGlobal.isEmpty()) {
        return;
    }

    QFile file(globalPath());
    if (file.open(QFile::ReadOnly)) {
        auto data = file.readAll();
        documents::languageGlobal = QJsonDocument::fromJson(data);
        file.close();
    }

    if (documents::languageGlobal.isEmpty()) {
        ContextDialog::ok(QObject::tr("The format of the language configuration file is incorrect or damaged. "
                                      "Check that the file is released correctly. "
                                      "If it cannot be solved, reinstall the software to solve the problem"));
        qCritical() << QString("Failed, %0 jsonDoc is Empty.").arg(globalPath());
        abort();
    }
}

bool Language::recovery()
{
    return true;
}

QStringList Language::ids()
{
    Language::initialize();
    return documents::languageGlobal.object().keys();
}

QString Language::id(const QString &filePath)
{
    Language::initialize();
    auto jsonObj = documents::languageGlobal.object();
    QFileInfo info(filePath);
    for (auto id : ids()) {
        auto langObjChild = jsonObj.value(id).toObject();
        QJsonArray suffixArray = langObjChild.value(Key_2::get()->suffix).toArray();
        QJsonArray baseArray = langObjChild.value(Key_2::get()->base).toArray();

        for (auto suffix : suffixArray) {
            if (info.fileName().endsWith(suffix.toString()))
                return id;

            if (info.suffix() == suffix.toString())
                return id;
        }

        for (auto base : baseArray) {

            if (info.fileName() == base.toString()
                    || info.fileName().toLower() == base.toString().toLower()) {
                return id;
            }
        }
    }

    return "";
}

Language::ServerInfo Language::sever(const QString &id)
{
    auto idObj = documents::languageGlobal.object().value(id).toObject();
    auto server = idObj.value(Key_2::get()->server).toString();
    auto serverArguments = idObj.value(Key_2::get()->serverArguments).toArray();
    QStringList args;
    for (auto arg : serverArguments) {
        args << arg.toString();
    }
    return ServerInfo{server, args};
}

QMap<int, QString> Language::tokenWords(const QString &id)
{
    QMap<int, QString> result;
    auto idObj = documents::languageGlobal.object().value(id).toObject();
    auto tokenChildObj = idObj.value(Key_2::get()->tokenWords).toObject();

    for (auto key: tokenChildObj.keys()) {
        result[key.toInt()] = tokenChildObj.value(key).toString();
    }

    return result;
}

QSet<QString> Language::suffixs(const QString &id)
{
    QSet<QString> result;
    auto idObj = documents::languageGlobal.object().value(id).toObject();
    auto suffixArray = idObj.value(Key_2::get()->suffix).toArray();
    for (auto value : suffixArray) {
        result.insert(value.toString());
    }
    return result;
}

QSet<QString> Language::bases(const QString &id)
{
    QSet<QString> result;
    auto idObj = documents::languageGlobal.object().value(id).toObject();
    auto baseArray = idObj.value(Key_2::get()->base).toArray();
    for (auto value : baseArray) {
        result.insert(value.toString());
    }
    return result;
}

} // namespace support_file
