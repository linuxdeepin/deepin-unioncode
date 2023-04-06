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
#ifndef TOOLCHAINDATA_H
#define TOOLCHAINDATA_H

#include <QSet>
#include <QMap>
#include <QMetaType>

namespace  {
static const QString kCCompilers{"C compilers"};
static const QString kCXXCompilers{"C++ compilers"};
static const QString kCCXXDebuggers{"C/C++ debuggers"};
static const QString kCCXXBuildSystems{"C/C++ build systems"};
static const QString kJDK{"JDK"};
static const QString kMaven{"Maven"};
static const QString kGradle{"Gradle"};
static const QString kPython{"Python"};
static const QString kNinja{"Ninja"};
static const QString kJS{"JS"};

static const QString kNameItem{"name"};
static const QString kPathItem{"path"};
}

class ToolChainData
{
public:
    struct ToolChainParam
    {
        QString name;
        QString path;
    };
    using Params = QVector<ToolChainParam>;

    // ToolChain type & Parameters.
    using ToolChains = QMap<QString, Params>;

    ToolChainData();

    const ToolChains &getToolChanins() const;
    bool readToolChainData(QString &retMsg);

private:    
    bool readToolChain(QString &filePath);
    ToolChains toolChains;
};

Q_DECLARE_METATYPE(ToolChainData::ToolChainParam);

#endif // TOOLCHAINDATA_H
