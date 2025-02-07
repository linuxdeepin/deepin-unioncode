// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "base/abstractinlinecompletionprovider.h"

#include <functional>

class ResourceManagerPrivate;
class ResourceManager
{
public:
    static ResourceManager *instance();

    void registerInlineCompletionProvider(AbstractInlineCompletionProvider *provider);
    QList<AbstractInlineCompletionProvider *> inlineCompletionProviders() const;

    using RepairCallback = std::function<void(const QString &info)>;
    void registerDiagnosticRepairTool(const QString &toolName, RepairCallback callback);
    QMap<QString, RepairCallback> getDiagnosticRepairTool() const;

private:
    ResourceManager();
    ~ResourceManager();

    ResourceManagerPrivate *const d;
};

#endif   // RESOURCEMANAGER_H
