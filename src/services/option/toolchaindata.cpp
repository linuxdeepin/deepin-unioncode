/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "toolchaindata.h"
#include "common/util/custompaths.h"
#include "common/toolchain/toolchain.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QFile>
#include <QJsonArray>
#include <QDebug>
#include <QDir>

ToolChainData::ToolChainData()
{
}

bool ToolChainData::readToolChain(QString &filePath)
{
    // Read all contents from toolchain file.
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    file.close();

    // Parse.
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qInfo() << "Toolchain file read failed!";
        return false;
    }

    auto parseSubObj = [this](QJsonObject &obj, const QString &subobjName) {
        if (obj.contains(subobjName)) {
            QJsonValue cCompilersArray = obj.value(subobjName);
            QJsonArray array = cCompilersArray.toArray();

            QStringList parameters;
            Params params;
            for (int i = 0; i < array.size(); ++i) {
                QJsonValue sub = array.at(i);
                QJsonObject subObj = sub.toObject();

                QJsonValue nameVal = subObj.value(kNameItem);
                QString name = nameVal.toString();

                QJsonValue pathVal = subObj.value(kPathItem);
                QString path = pathVal.toString();

                ToolChainParam param;
                param.name = name;
                param.path = path;

                params.push_back(param);
            }
            toolChains.insert(subobjName, params);
        }
    };

    QJsonArray array = doc.array();
    for (auto node : array) {
        auto obj = node.toObject();
        for (auto it = obj.begin(); it != obj.end(); it++) {
            parseSubObj(obj, it.key());
        }
    }

    return true;
}

const ToolChainData::ToolChains &ToolChainData::getToolChanins() const
{
    return toolChains;
}

bool ToolChainData::readToolChainData(QString &retMsg)
{
    bool ret = false;
    QString toolChainFilePath = CustomPaths::user(CustomPaths::Configures)
            + QDir::separator() + toolchains::K_TOOLCHAINFILE;
    if (QFile(toolChainFilePath).exists()) {
        ret = readToolChain(toolChainFilePath);
        retMsg = ret ? "read tool chain successful!" : "read tool chain failed!";
    }

    return ret;
}
