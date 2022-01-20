#include "supportfile.h"
#include "custompaths.h"
#include "dialog/contextdialog.h"

#include <QStringList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QtConcurrent>

#define GLOBAL_ERROR 1
#define USER_ERROR 2

namespace SupportFilePrivate{
struct Documents {
    inline static QJsonDocument builderGlobal;
    inline static QJsonDocument builderUser;
    inline static QJsonDocument languageGlobal;
    inline static QJsonDocument languageUser;
    inline static QHash<QString, QJsonDocument> editorStyleGlobal;
    inline static QHash<QString, QJsonDocument> editorStyleUser;
    inline static QJsonDocument windowStyleGlobal;
    inline static QJsonDocument windowStyleUser;
    static bool builderIsLoaded();
    static bool languageIsLoaded();
    static bool editorStyleIsLoaded(const QString &languageID);
    static bool windowStyleIsLoaded();
    static int loadDocument(QJsonDocument &globalDoc,
                            const QString &globalFilePath,
                            QJsonDocument &userDoc,
                            const QString &userFilePath,
                            QString * errorString = nullptr);
    static SupportFile::BuildFileInfo getBuildFileInfo(const QJsonDocument &doc,
                                                       const QFileInfo &info);
};
} using namespace SupportFilePrivate;

QString SupportFile::Builder::globalPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Configures));
    return result + "builder.support";
}

QString SupportFile::Builder::userPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::user(CustomPaths::Configures));
    return result + "builder.support";
}

void SupportFile::Builder::initialize()
{
    if (!Documents::builderIsLoaded()) {
        QString error;
        if( 0 != Documents::loadDocument(Documents::builderGlobal,
                                         SupportFile::Builder::globalPath(),
                                         Documents::builderUser,
                                         SupportFile::Builder::userPath(),
                                         &error)) {
            ContextDialog::ok(error);
        }
    }
}

QString SupportFile::Builder::buildSystem(const QString &filePath)
{
    SupportFile::Builder::initialize();

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists())
        return "";

    QJsonObject globalJsonObj = Documents::builderGlobal.object();
    QStringList globalJsonObjKeys = globalJsonObj.keys();
    foreach (auto val, globalJsonObjKeys) {
        if (globalJsonObj.value(val).toObject().value("suffix").toArray().contains(fileInfo.suffix()))
            return val;
        if (globalJsonObj.value(val).toObject().value("base").toArray().contains(fileInfo.fileName()))
            return val;
    }

    QJsonObject cacheJsonObj = Documents::builderUser.object();
    QStringList cacheJsonObjKeys = cacheJsonObj.keys();
    foreach (auto val, cacheJsonObjKeys) {
        if (cacheJsonObj.value(val).toObject().value("suffix").toArray().contains(fileInfo.suffix()))
            return val;
        if (cacheJsonObj.value(val).toObject().value("base").toArray().contains(fileInfo.fileName()))
            return val;
    }

    return "";
}

SupportFile::BuildFileInfo SupportFile::Builder::buildInfo(const QString &filePath)
{
    QFileInfo info(filePath);
    if (!info.exists())
        return {};

    SupportFile::BuildFileInfo result = Documents::getBuildFileInfo(Documents::builderUser, info);
    if (result.isEmpty())
        result = Documents::getBuildFileInfo(Documents::builderGlobal, info);
    return result;
}

QList<SupportFile::BuildFileInfo> SupportFile::Builder::buildInfos(const QString &dirPath)
{
    SupportFile::Builder::initialize();

    if (Documents::builderIsLoaded()) {
        QSet<BuildFileInfo> result;
        QDir dir(dirPath);
        QFileInfoList infos = dir.entryInfoList(QDir::Filter::NoDot|QDir::Filter::NoDotDot|QDir::Filter::Files);
        QJsonObject globalObject = Documents::builderGlobal.object();
        QJsonObject userObject = Documents::builderUser.object();

        auto mappedFunc = [=, &result](const QFileInfo &info)
        {
            if (!info.exists())
                return;

            auto tempBuildInfo = Documents::getBuildFileInfo(Documents::builderGlobal, info);
            if (!tempBuildInfo.isEmpty())
                result += tempBuildInfo;

            tempBuildInfo = Documents::getBuildFileInfo(Documents::builderUser, info);
            if (!tempBuildInfo.isEmpty())
                result += tempBuildInfo;
        };

        QtConcurrent::blockingMap(infos, mappedFunc);

        return result.toList();
    }

    return {};
}

QString SupportFile::Language::globalPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Configures));
    return result + "language.support";
}

QString SupportFile::Language::userPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::user(CustomPaths::Configures));
    return result + "language.support";
}

QString SupportFile::EditorStyle::globalPath(const QString &languageID)
{
    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Configures));
    return result + QString("editorstyle_%0.support").arg(languageID);
}

QString SupportFile::EditorStyle::userPath(const QString &languageID)
{
    auto result = CustomPaths::endSeparator(CustomPaths::user(CustomPaths::Configures));
    return result + QString("editorstyle_%0.support").arg(languageID);
}

QString SupportFile::WindowStyle::globalPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Configures));
    return result + QString("windowstyle.support");
}

QString SupportFile::WindowStyle::userPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::user(CustomPaths::Configures));
    return result + QString("windowstyle.support");
}

bool SupportFilePrivate::Documents::builderIsLoaded()
{
    return !builderGlobal.isEmpty() && !builderUser.isEmpty();
}

bool SupportFilePrivate::Documents::languageIsLoaded()
{
    return !languageGlobal.isEmpty() && !languageUser.isEmpty();
}

bool SupportFilePrivate::Documents::editorStyleIsLoaded(const QString &languageID)
{
    if (languageID.isEmpty())
        return false;

    return !editorStyleGlobal.value(languageID).isEmpty()
            && !editorStyleUser.value(languageID).isEmpty();
}

bool SupportFilePrivate::Documents::windowStyleIsLoaded()
{
    return !windowStyleGlobal.isEmpty() && !windowStyleGlobal.isEmpty();
}

int SupportFilePrivate::Documents::loadDocument(QJsonDocument &globalDoc,
                                                const QString &globalFilePath,
                                                QJsonDocument &userDoc,
                                                const QString &userFilePath,
                                                QString *errorString)
{
    QFile file(globalFilePath);
    if (!file.exists()) {
        if (errorString)
            *errorString = QString("Failed, not found global configure file: %0").arg(globalFilePath);
        return GLOBAL_ERROR;
    }

    if (!file.open(QFile::ReadOnly)) {
        if (errorString)
            *errorString = QString("Failed, can't open global configure file: %0").arg(globalFilePath);
        return GLOBAL_ERROR;
    }

    globalDoc = QJsonDocument::fromJson(file.readAll());
    file.close();

    CustomPaths::checkDir(CustomPaths::user(CustomPaths::Configures));

    QFileInfo fileInfo(userFilePath);
    if (!fileInfo.exists()) {
        QFile::copy(globalFilePath, fileInfo.filePath());
    }

    QFile userfile(userFilePath);
    if (!userfile.permissions().testFlag(QFile::WriteUser)) {
        userfile.setPermissions(userfile.permissions() | QFile::WriteUser);
    }

    if (!userfile.open(QFile::OpenModeFlag::ReadOnly)) {
        if (errorString) {
            *errorString += "Failed, can't open user configure file: ";
            *errorString += userFilePath;
            return USER_ERROR;
        }
    }

    userDoc = QJsonDocument::fromJson(userfile.readAll());
    userfile.close();
    return 0;
}

SupportFile::BuildFileInfo Documents::getBuildFileInfo(const QJsonDocument &doc, const QFileInfo &info)
{
    SupportFile::BuildFileInfo result;
    auto docObject = doc.object();
    auto keys = docObject.keys();
    for (auto key : keys) {
        QJsonObject keyObject = docObject.value(key).toObject();
        QJsonArray suffixs = keyObject.value("suffix").toArray();
        QJsonArray bases = keyObject.value("base").toArray();
        foreach (auto suffix, suffixs) {
            if (suffix == info.suffix()) {
                return SupportFile::BuildFileInfo { key, info.filePath() };
            }
        }

        foreach (auto base, bases) {
            if (base == info.fileName()) {
                return SupportFile::BuildFileInfo { key, info.filePath() };
            }
        }
    }
    return result;
}

uint qHash(const SupportFile::BuildFileInfo &info, uint seed)
{
    return qHash(info.buildSystem + " " + info.projectPath, seed);
}

bool SupportFile::BuildFileInfo::operator==(const SupportFile::BuildFileInfo &info) const
{
    return buildSystem == info.buildSystem
            && projectPath == info.projectPath;
}

bool SupportFile::BuildFileInfo::isEmpty()
{
    return buildSystem.isEmpty() || projectPath.isEmpty();
}
