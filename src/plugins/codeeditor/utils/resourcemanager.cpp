// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resourcemanager.h"
#include "base/abstractinlinecompletionprovider.h"

#include <QDebug>

class ResourceManagerPrivate
{
public:
    QList<AbstractInlineCompletionProvider *> providerList;
    QMap<QString, ResourceManager::RepairCallback> repairToolInfo;
};

ResourceManager::ResourceManager()
    : d(new ResourceManagerPrivate)
{
}

ResourceManager::~ResourceManager()
{
    delete d;
}

ResourceManager *ResourceManager::instance()
{
    static ResourceManager ins;
    return &ins;
}

void ResourceManager::registerInlineCompletionProvider(AbstractInlineCompletionProvider *provider)
{
    if (d->providerList.contains(provider)) {
        qWarning() << "This provider has registered, name: " << provider->providerName();
        return;
    }

    d->providerList << provider;
}

QList<AbstractInlineCompletionProvider *> ResourceManager::inlineCompletionProviders() const
{
    return d->providerList;
}

void ResourceManager::registerDiagnosticRepairTool(const QString &toolName, RepairCallback callback)
{
    if (d->repairToolInfo.contains(toolName) || !callback)
        return;

    d->repairToolInfo.insert(toolName, callback);
}

QMap<QString, ResourceManager::RepairCallback> ResourceManager::getDiagnosticRepairTool() const
{
    return d->repairToolInfo;
}
