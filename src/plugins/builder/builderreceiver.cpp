/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#include "builderreceiver.h"
#include "common/util/eventdefinitions.h"
#include "buildersignals.h"
#include "builderglobals.h"

static QStringList subTopics{T_MENU, T_FILEBROWSER};
typedef QMap<ToolChainType, QString> MAPToolChainType;
static MAPToolChainType suffixMimeTypes{{ToolChainType::QMake, "pro"}};
static MAPToolChainType fullMatchTypes{{ToolChainType::CMake, "CMakeLists.txt"}};
static const char *const outputDirName = "build";

static BuilderReceiver *ins = nullptr;

BuilderReceiver::BuilderReceiver(QObject *parent)
    : dpf::EventHandler(parent)
    , dpf::AutoEventHandlerRegister<BuilderReceiver>()
{
    ins = this;
}

BuilderReceiver *BuilderReceiver::instance()
{
    return ins;
}

dpf::EventHandler::Type BuilderReceiver::type()
{
    return dpf::EventHandler::Type::Async;
}

QStringList &BuilderReceiver::topics()
{
    return subTopics;
}

const QString &BuilderReceiver::projectFilePath() const
{
    return proFilePath;
}

const QString &BuilderReceiver::projectDirectory() const
{
    return proDirPath;
}

const QString &BuilderReceiver::rootProjectDirectory() const
{
    return proRootPath;
}

const QString &BuilderReceiver::buildOutputDirectory() const
{
    return buildOutputPath;
}

ToolChainType BuilderReceiver::toolChainType() const
{
    return tlChainType;
}

void BuilderReceiver::eventProcess(const dpf::Event &event)
{
    if (!topics().contains(event.topic())) {
        qDebug() << "Fatal";
        return;
    }

    qInfo() << event;

    QString topic = event.topic();
    QString data = event.data().toString();
    if (topic == T_MENU) { // TODO(mozart):Menu event should contain more info.
        if (data == D_FILE_OPENFOLDER) { // more events may be here.
            QString filePath = event.property(P_FILEPATH).toString();
            proDirPath = filePath;
            updatePaths(filePath);
        }
    } else if (topic == T_FILEBROWSER) {
        if (data == D_ITEM_MENU_BUILD) { // more events may be here.
            proFilePath = event.property(P_BUILDFILEPATH).toString();
            proDirPath = QFileInfo(proFilePath).dir().path();
            proRootPath = proDirPath; // TODO(mozart):path should get from event in the future.
            buildOutputPath = event.property(P_BUILDDIRECTORY).toString();
            emit builderSignals->buildTriggered();
        }
    }
}

void BuilderReceiver::updatePaths(const QString &path)
{
    QDir dir(path);
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::DirsLast);

    QFileInfoList list = dir.entryInfoList();
    bool bFounded = false;
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);

        // directory loop find.
        if (fileInfo.isDir() && fileInfo.fileName() != "." && fileInfo.fileName() != "..") {
            updatePaths(fileInfo.filePath());
        }

        QString fileName = fileInfo.fileName();
        for (auto type : fullMatchTypes) {
            if (!QString::compare(fileName, type, Qt::CaseInsensitive)) {
                bFounded = true;
                if (type == fullMatchTypes[ToolChainType::CMake])
                    tlChainType = ToolChainType::CMake;
                break;
            }
        }

        QString suffix = fileInfo.suffix();
        for (auto type : suffixMimeTypes) {
            if (!QString::compare(suffix, type, Qt::CaseInsensitive)) {
                bFounded = true;
                if (type == suffixMimeTypes[ToolChainType::QMake])
                    tlChainType = ToolChainType::QMake;
                break;
            }
        }

        if (bFounded) {
            proFilePath = fileInfo.filePath();
            proRootPath = fileInfo.dir().path();
            buildOutputPath = proDirPath + "/" + outputDirName;

            qInfo() << "project file path : " << proFilePath;
            qInfo() << "project root path : " << proRootPath;
            break;
        }
    }
}

