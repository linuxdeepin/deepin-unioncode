// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kitmanager.h"

#include "services/option/toolchaindata.h"

#include <QUuid>
#include <QDebug>

KitManager::KitManager(QObject *parent)
    : QObject(parent)
{
}

KitManager *KitManager::instance()
{
    static KitManager ins;
    return &ins;
}

void KitManager::setKitList(const QList<Kit> &list)
{
    allKit = list;
}

void KitManager::removeKit(const Kit &kit)
{
    allKit.removeOne(kit);
}

QList<Kit> KitManager::kitList() const
{
    return allKit;
}

Kit KitManager::findKit(const QString &id)
{
    auto iter = std::find_if(allKit.cbegin(), allKit.cend(),
                             [&id](const Kit &kit) {
                                 return kit.id() == id;
                             });

    return iter != allKit.cend() ? *iter : Kit();
}

Kit KitManager::defaultKit() const
{
    Kit kit;

    ToolChainData toolCD;
    QString retMsg;
    bool ret = toolCD.readToolChainData(retMsg);
    if (!ret) {
        qWarning() << retMsg;
        return kit;
    }

    const auto &data = toolCD.getToolChanins();
    auto params = data.value(kCCompilers);
    for (const auto &param : qAsConst(params)) {
        kit.setCCompiler({ param.name, param.path });
        if (param.name == "gcc")
            break;
    }

    params = data.value(kCXXCompilers);
    for (const auto &param : qAsConst(params)) {
        kit.setCXXCompiler({ param.name, param.path });
        if (param.name == "g++")
            break;
    }

    params = data.value(kCCXXDebuggers);
    for (const auto &param : qAsConst(params)) {
        kit.setDebugger({ param.name, param.path });
        if (param.name == "gdb")
            break;
    }

    params = data.value(kCCXXBuildSystems);
    for (const auto &param : qAsConst(params)) {
        kit.setCMakeTool({ param.name, param.path });
        if (param.name == "cmake")
            break;
    }

    kit.setKitName("Desktop");
    kit.setId(QUuid::createUuid().toString());
    return kit;
}
