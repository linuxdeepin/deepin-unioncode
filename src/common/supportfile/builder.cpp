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
#include "builder.h"
#include "util/custompaths.h"
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

#define NO_ERROR 0
#define GLOBAL_ERROR 1
#define USER_ERROR 2
namespace support_file {
namespace documents {
inline static QJsonDocument builderGlobal;
inline static QJsonDocument builderUser;

static bool builderIsLoaded();

static int loadDocument(QJsonDocument &globalDoc, const QString &globalFilePath,
                        QJsonDocument &userDoc, const QString &userFilePath,
                        QString * errorString = nullptr);

static BuildFileInfo getBuildFileInfo(const QJsonDocument &doc,
                                      const QFileInfo &info);
}

QString Builder::globalPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::global(CustomPaths::Configures));
    return result + "builder.support";
}

QString Builder::userPath()
{
    auto result = CustomPaths::endSeparator(CustomPaths::user(CustomPaths::Configures));
    return result + "builder.support";
}

void Builder::initialize()
{
    if (!documents::builderIsLoaded()) {
        QString error;
        if (0 != documents::loadDocument(documents::builderGlobal,
                                         support_file::Builder::globalPath(),
                                         documents::builderUser,
                                         support_file::Builder::userPath(),
                                         &error)) {
            ContextDialog::ok(error);
        }
    }
}

QString Builder::buildSystem(const QString &filePath)
{
    support_file::Builder::initialize();

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists())
        return "";

    QJsonObject globalJsonObj = documents::builderGlobal.object();
    QStringList globalJsonObjKeys = globalJsonObj.keys();
    foreach (auto val, globalJsonObjKeys) {
        if (globalJsonObj.value(val).toObject().value("suffix").toArray().contains(fileInfo.suffix()))
            return val;
        if (globalJsonObj.value(val).toObject().value("base").toArray().contains(fileInfo.fileName()))
            return val;
    }

    QJsonObject cacheJsonObj = documents::builderUser.object();
    QStringList cacheJsonObjKeys = cacheJsonObj.keys();
    foreach (auto val, cacheJsonObjKeys) {
        if (cacheJsonObj.value(val).toObject().value("suffix").toArray().contains(fileInfo.suffix()))
            return val;
        if (cacheJsonObj.value(val).toObject().value("base").toArray().contains(fileInfo.fileName()))
            return val;
    }

    return "";
}

BuildFileInfo Builder::buildInfo(const QString &filePath)
{
    QFileInfo info(filePath);
    if (!info.exists())
        return {};

    support_file::BuildFileInfo result = documents::getBuildFileInfo(documents::builderUser, info);
    if (result.isEmpty())
        result = documents::getBuildFileInfo(documents::builderGlobal, info);
    return result;
}

QList<BuildFileInfo> Builder::buildInfos(const QString &dirPath)
{
    support_file::Builder::initialize();

    if (documents::builderIsLoaded()) {
        QSet<BuildFileInfo> result;
        QDir dir(dirPath);
        QFileInfoList infos = dir.entryInfoList(QDir::Filter::NoDot|QDir::Filter::NoDotDot|QDir::Filter::Files);
        QJsonObject globalObject = documents::builderGlobal.object();
        QJsonObject userObject = documents::builderUser.object();

        auto mappedFunc = [=, &result](const QFileInfo &info)
        {
            if (!info.exists())
                return;

            auto tempBuildInfo = documents::getBuildFileInfo(documents::builderGlobal, info);
            if (!tempBuildInfo.isEmpty())
                result += tempBuildInfo;

            tempBuildInfo = documents::getBuildFileInfo(documents::builderUser, info);
            if (!tempBuildInfo.isEmpty())
                result += tempBuildInfo;
        };

        QtConcurrent::blockingMap(infos, mappedFunc);

        return result.values();
    }

    return {};
}



bool support_file::documents::builderIsLoaded()
{
    return !builderGlobal.isEmpty() && !builderUser.isEmpty();
}

int support_file::documents::loadDocument(QJsonDocument &globalDoc,
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
    return NO_ERROR;
}

BuildFileInfo documents::getBuildFileInfo(const QJsonDocument &doc, const QFileInfo &info)
{
    support_file::BuildFileInfo result;
    auto docObject = doc.object();
    auto keys = docObject.keys();
    for (auto key : keys) {
        QJsonObject keyObject = docObject.value(key).toObject();
        QJsonArray suffixs = keyObject.value("suffix").toArray();
        QJsonArray bases = keyObject.value("base").toArray();
        foreach (auto suffix, suffixs) {
            if (suffix == info.suffix()) {
                return support_file::BuildFileInfo { key, info.filePath() };
            }
        }

        foreach (auto base, bases) {
            if (base == info.fileName()) {
                return support_file::BuildFileInfo { key, info.filePath() };
            }
        }
    }
    return result;
}

bool BuildFileInfo::operator==(const support_file::BuildFileInfo &info) const
{
    return buildSystem == info.buildSystem
            && projectPath == info.projectPath;
}

bool BuildFileInfo::isEmpty()
{
    return buildSystem.isEmpty() || projectPath.isEmpty();
}

uint qHash(const support_file::BuildFileInfo &info, uint seed)
{
    return qHash(info.buildSystem + " " + info.projectPath, seed);
}

} // namespace support_file


