// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stylelsppython.h"

#include "textedittabwidget/textedit.h"
#include "textedittabwidget/style/stylecolor.h"
#include "textedittabwidget/style/stylejsonfile.h"

StyleLspPython::StyleLspPython(TextEdit *parent)
    : StyleLsp (parent)
{

}


StyleLsp::IndicStyleExt StyleLspPython::symbolIndic(lsp::SemanticTokenType::type_value token,
                                                    QList<lsp::SemanticTokenModifier::type_index> modifier)
{
    Q_UNUSED(modifier);
    IndicStyleExt result;

    auto jsonFile = edit()->getStyleFile();
    if (jsonFile->setTheme(StyleJsonFile::Theme::get()->Dark)) {
        QJsonObject tempObj;
        int tempFore = 0;
        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Self).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        if (lsp::SemanticTokenType::get()->Namespace == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Namespace).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Type == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Type).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Class == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Class).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Enum == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Enumeration).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Interface == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Struct == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->TypeParameter == token) {
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Variable).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Parameter == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Variable == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Variable).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Property == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Property).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->EnumMember == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Enumeration).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Event == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Function == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Function).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Method == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->FunctionDeclaration).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Macro == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Macro).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Keyword == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Keyword).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Modifier == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Comment == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Comment).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->String == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->String).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Number == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Number).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Regexp == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->String).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        if (lsp::SemanticTokenType::get()->Operator == token){
            tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Operators).toObject();
            tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        }
        result.fore[INDIC_TEXTFORE] = tempFore;
    }
    return result;
}


