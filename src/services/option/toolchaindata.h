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

static const char *kCCompilers = "C compilers";
static const char *kCXXCompilers = "C++ compilers";
static const char *kCCXXDebuggers = "C/C++ debuggers";
static const char *kCCXXBuildSystems = "C/C++ build systems";
static const char *kJDK = "JDK";
static const char *kMaven = "Maven";
static const char *kGradle = "Gradle";
static const char *kPython = "Python";

static const char *kNameItem = "name";
static const char *kPathItem = "path";

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
