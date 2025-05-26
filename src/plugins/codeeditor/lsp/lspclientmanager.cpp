// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lspclientmanager.h"

#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

LSPClientManager::LSPClientManager()
{
}

LSPClientManager::~LSPClientManager()
{
    qDeleteAll(clientHash.values());
}

LSPClientManager *LSPClientManager::instance()
{
    static LSPClientManager ins;
    return &ins;
}

newlsp::Client *LSPClientManager::get(const newlsp::ProjectKey &key)
{
    if (!key.isValid())
        return nullptr;

    if (clientHash.contains(key)) {
        qApp->metaObject()->invokeMethod(clientHash[key], "selectLspServer", Q_ARG(const newlsp::ProjectKey &, key));
    } else {
        auto client = new newlsp::Client();
        qApp->metaObject()->invokeMethod(client, "selectLspServer", Q_ARG(const newlsp::ProjectKey &, key));
        qApp->metaObject()->invokeMethod(client, "initRequest");
        clientHash.insert(key, client);
    }

    return clientHash[key];
}

QColor LSPClientManager::highlightColor(const QString &langId, lsp::SemanticTokenType::type_value token)
{
    auto defaultColor = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType
            ? QColor("#000000")
            : QColor("#d6cf9a");

    QMutexLocker lk(&dataMutex);
    if (!styleMap.contains(langId)) {
        const auto &filePath = support_file::EditorStyle::globalPath(langId);
        StyleSettings settings;
        if (!settings.load(filePath))
            return defaultColor;

        styleMap.insert(langId, settings);
    }

    QString theme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType
            ? StyleSettings::Theme::get()->Light
            : StyleSettings::Theme::get()->Dark;

    QJsonObject typeObj;
    if (lsp::SemanticTokenType::get()->Namespace == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Namespace).toObject();
    } else if (lsp::SemanticTokenType::get()->Type == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Type).toObject();
    } else if (lsp::SemanticTokenType::get()->Class == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Class).toObject();
    } else if (lsp::SemanticTokenType::get()->Enum == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Enumeration).toObject();
    } else if (lsp::SemanticTokenType::get()->Interface == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Text).toObject();
    } else if (lsp::SemanticTokenType::get()->Struct == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Text).toObject();
    } else if (lsp::SemanticTokenType::get()->TypeParameter == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Variable).toObject();
    } else if (lsp::SemanticTokenType::get()->Parameter == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Parameter).toObject();
    } else if (lsp::SemanticTokenType::get()->Variable == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Variable).toObject();
    } else if (lsp::SemanticTokenType::get()->Property == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Property).toObject();
    } else if (lsp::SemanticTokenType::get()->EnumMember == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Enumeration).toObject();
    } else if (lsp::SemanticTokenType::get()->Event == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Text).toObject();
    } else if (lsp::SemanticTokenType::get()->Function == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Function).toObject();
    } else if (lsp::SemanticTokenType::get()->Method == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->FunctionDeclaration).toObject();
    } else if (lsp::SemanticTokenType::get()->Macro == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Macro).toObject();
    } else if (lsp::SemanticTokenType::get()->Keyword == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Keyword).toObject();
    } else if (lsp::SemanticTokenType::get()->Modifier == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Keyword).toObject();
    } else if (lsp::SemanticTokenType::get()->Comment == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Comment).toObject();
    } else if (lsp::SemanticTokenType::get()->String == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->String).toObject();
    } else if (lsp::SemanticTokenType::get()->Number == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Number).toObject();
    } else if (lsp::SemanticTokenType::get()->Regexp == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->String).toObject();
    } else if (lsp::SemanticTokenType::get()->Operator == token) {
        typeObj = styleMap[langId].value(theme, StyleSettings::Key_1::get()->Operators).toObject();
    }

    if (typeObj.isEmpty())
        return defaultColor;

    auto colorName = typeObj.value(StyleSettings::Key_2::get()->Foreground).toString();
    if (!colorName.isEmpty() && colorName.at(0) != '#')
        colorName.prepend("#");

    return QColor(colorName);
}
