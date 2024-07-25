// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
