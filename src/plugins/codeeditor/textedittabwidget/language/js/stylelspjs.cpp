/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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


