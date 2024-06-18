// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LSPCLIENTMANAGER_H
#define LSPCLIENTMANAGER_H

#include "stylesettings.h"
#include "common/common.h"

class LSPClientManager
{
public:
    static LSPClientManager *instance();

    newlsp::Client *get(const newlsp::ProjectKey &key);
    QColor highlightColor(const QString &langId, lsp::SemanticTokenType::type_value token);

private:
    explicit LSPClientManager();
    ~LSPClientManager();

    QHash<newlsp::ProjectKey, newlsp::Client *> clientHash;
    QMap<QString, StyleSettings> styleMap;
};

#endif   // LSPCLIENTMANAGER_H
