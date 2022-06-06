/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
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
#include "stylelspcpp.h"
#include "textedittabwidget/textedit.h"
#include "textedittabwidget/style/stylecolor.h"
#include "textedittabwidget/style/stylejsonfile.h"
#include "common/common.h"

#include <QDir>
#include <QJsonObject>
#include <QRegularExpression>


namespace {
static bool checkVersionOk = false;
}

StyleLspCpp::StyleLspCpp(TextEdit *parent)
 : StyleLsp (parent)
{

}

StyleLsp::ServerInfo StyleLspCpp::clientInfoSpec(StyleLsp::ServerInfo info)
{
    QString customClangd = QString(LIBRARY_INSTALL_PREFIX) + QDir::separator() + "tools"
            + QDir::separator() + info.progrma;
    QFileInfo customClangdInfo(customClangd);
    qInfo() << "custom clangd path:" << customClangdInfo.filePath();
    qInfo() << "source clangd path:" << info.progrma;

    // 优先使用自定义环境
    if (customClangdInfo.exists() && customClangdInfo.isExecutable()) {
        return { customClangd, info.arguments };
    }

    if (ProcessUtil::exists(info.progrma)) {
        QRegularExpression regExp("[0-9]*\\.[0-9]*\\.[0-9]*");
        auto versionMatchs = regExp.match(ProcessUtil::version(info.progrma)).capturedTexts();
        for (auto versionMatch : versionMatchs) {
            QStringList versions = versionMatch.split(".");
            if (versions.size() == 3) {
                auto major =  versions[0];
                if (major.toInt() < 10 && !::checkVersionOk) {
                    checkVersionOk = true;
                    QString mess = lsp::Client::tr("clangd lower verion: 10 from %0.\n"
                                                   "Does not meet the current operating environment");
                    mess += QString("\n%0").arg(info.progrma);
                    mess += QString("\n%0").arg(customClangd);
                    ContextDialog::ok(mess);
                }
            }
        }
    }

    return info;
}

StyleLsp::IndicStyleExt StyleLspCpp::symbolIndic(lsp::SemanticTokenType::type_value token,
                                                 QList<lsp::SemanticTokenModifier::type_index> modifier)
{
    Q_UNUSED(modifier);
    IndicStyleExt result;

    auto jsonFile = edit()->getStyleFile();
    if (jsonFile->setTheme(StyleJsonFile::Theme::get()->Dark)) {
        QJsonObject tempObj;
        int tempFore = 0;
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
