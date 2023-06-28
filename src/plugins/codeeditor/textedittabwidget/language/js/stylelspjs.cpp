// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stylelspjs.h"

#include "textedittabwidget/textedit.h"
#include "textedittabwidget/style/stylecolor.h"
#include "textedittabwidget/style/stylejsonfile.h"

StyleLspJS::StyleLspJS(TextEdit *parent)
    : StyleLsp (parent)
{

}

StyleLsp::IndicStyleExt StyleLspJS::symbolIndic(lsp::SemanticTokenType::type_value token,
                                                    QList<lsp::SemanticTokenModifier::type_index> modifier)
{
    Q_UNUSED(modifier);
    IndicStyleExt result;

    auto jsonFile = edit()->getStyleFile();
    if (jsonFile->setTheme(StyleJsonFile::Theme::get()->Dark)) {
        QJsonObject tempObj;
        int tokenForeColor = 0;
        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Self).toObject();
        tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        if (lsp::SemanticTokenType::get()->Namespace == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Namespace).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Type == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Type).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Class == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Class).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Enum == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Enumeration).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Interface == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Struct == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->TypeParameter == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Variable).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Parameter == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Variable == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Variable).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Property == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Property).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->EnumMember == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Enumeration).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Event == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Function == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Function).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Method == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->FunctionDeclaration).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Macro == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Macro).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Keyword == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Keyword).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Modifier == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Comment == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Comment).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->String == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->String).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Number == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Number).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Regexp == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->String).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Operator == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Operators).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else if (lsp::SemanticTokenType::get()->Member == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Member).toObject();
            tokenForeColor = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        } else {
            result.fore[INDIC_TEXTFORE] = tokenForeColor;
        }
    }
    return result;
}


