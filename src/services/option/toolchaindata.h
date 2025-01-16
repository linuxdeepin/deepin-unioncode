// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLCHAINDATA_H
#define TOOLCHAINDATA_H

#include "services/services_global.h"

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

class SERVICE_EXPORT ToolChainData
{
public:
    struct ToolChainParam
    {
        QString name;
        QString path;
        bool operator ==(const ToolChainParam &param) const {
            return (this->name == param.name && this->path == param.path);
        }
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
