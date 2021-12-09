/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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

static QStringList subTopics{"Menu", "FileBrowser"};
typedef QMap<ToolChainType, QString> MAPToolChainType;
static MAPToolChainType suffixMimeTypes{{ToolChainType::QMake, "pro"}};
static MAPToolChainType fullMatchTypes{{ToolChainType::CMake, "CMakeLists.txt"}};

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

QStringList BuilderReceiver::topics()
{
    return subTopics;
}

QString BuilderReceiver::projectFilePath() const
{
    return proFilePath;
}

QString BuilderReceiver::projectDirectory() const
{
    return proDirPath;
}

QString BuilderReceiver::rootProjectDirectory() const
{
    return proRootPath;
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

    QString data = event.data().toString();
    if (event.topic() == "Menu") {
        if (event.data() == "File.OpenFolder") {
            QString filePath = event.property("FilePath").toString();
            proDirPath = filePath;
            updatePaths(filePath);
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
            qInfo() << "project file path : " << proFilePath;
            qInfo() << "project root path : " << proRootPath;
            break;
        }
    }
}

