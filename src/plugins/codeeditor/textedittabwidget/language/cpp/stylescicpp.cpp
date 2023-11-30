// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stylescicpp.h"
#include "textedittabwidget/style/stylejsonfile.h"
#include "textedittabwidget/textedit.h"
#include "textedittabwidget/style/stylecolor.h"

#include "SciLexer.h"


StyleSciCpp::StyleSciCpp(TextEdit *parent)
    : StyleSci(parent)
{

}

QMap<int, QString> StyleSciCpp::keyWords() const
{
    return {
        { 1, "and and_eq asm auto bitand bitor bool break case "
            "catch char class compl const const_cast continue "
            "default delete do double dynamic_cast else enum "
            "explicit export extern false float for friend goto if "
            "inline int long mutable namespace new not not_eq "
            "operator or or_eq private protected public register "
            "reinterpret_cast return short signed sizeof static "
            "static_cast struct switch template this throw true "
            "try typedef typeid typename union unsigned using "
            "virtual void volatile wchar_t while xor xor_eq"
        }
    };
}

void StyleSciCpp::setStyle()
{
    StyleSci::setStyle();
}

int StyleSciCpp::sectionEnd() const
{
    return SCE_C_ESCAPESEQUENCE; //default style key end;
}

void StyleSciCpp::setThemeColor(DGuiApplicationHelper::ColorType colorType)
{
    StyleSci::setThemeColor(colorType);

    if (colorType == DGuiApplicationHelper::DarkType) {
        QJsonObject tempObj;
        int tempFore;
        auto jsonFile = edit()->getStyleFile();
        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Comment).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_C_COMMENT, tempFore); // #整行
        edit()->styleSetFore(SCE_C_COMMENTLINE, tempFore); // //注释
        edit()->styleSetFore(SCE_C_COMMENTDOC, tempFore);
        edit()->styleSetFore(SCE_C_COMMENTLINEDOC, tempFore); // ///注释
        edit()->styleSetFore(SCE_C_COMMENTDOCKEYWORD, tempFore - 0x3333);
        edit()->styleSetFore(SCE_C_COMMENTDOCKEYWORDERROR, tempFore - 0x3333); // /// @
        edit()->styleSetFore(SCE_C_PREPROCESSORCOMMENT, tempFore);
        edit()->styleSetFore(SCE_C_PREPROCESSORCOMMENTDOC, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Number).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_C_NUMBER, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Keyword).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_C_WORD, tempFore);
        edit()->styleSetFore(SCE_C_WORD2, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->String).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_C_STRING, tempFore); // 字符串
        edit()->styleSetFore(SCE_C_CHARACTER, tempFore); // 字符串
        edit()->styleSetFore(SCE_C_UUID, tempFore);
        edit()->styleSetFore(SCE_C_STRINGEOL, tempFore);
        edit()->styleSetFore(SCE_C_REGEX, tempFore);
        edit()->styleSetFore(SCE_C_STRINGRAW, tempFore);
        edit()->styleSetFore(SCE_C_VERBATIM, tempFore);
        edit()->styleSetFore(SCE_C_HASHQUOTEDSTRING, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Preprocessor).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_C_PREPROCESSOR, tempFore); // #

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Operators).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_C_OPERATOR, tempFore); // 符号

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Text).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_C_IDENTIFIER, tempFore);
        edit()->styleSetFore(SCE_C_USERLITERAL, tempFore);
        edit()->styleSetFore(SCE_C_TASKMARKER, tempFore);
        edit()->styleSetFore(SCE_C_ESCAPESEQUENCE, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Global).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_C_GLOBALCLASS, tempFore);
    } else {
        edit()->styleSetFore(SCE_C_DEFAULT, StyleColor::color(StyleColor::Table::get()->DarkBlue)); // #整行
        edit()->styleSetFore(SCE_C_COMMENT, StyleColor::color(StyleColor::Table::get()->DeepSkyBlue)); // #整行
        edit()->styleSetFore(SCE_C_COMMENTLINE, StyleColor::color(StyleColor::Table::get()->DarkTurquoise)); // //注释
        edit()->styleSetFore(SCE_C_COMMENTDOC, StyleColor::color(StyleColor::Table::get()->DarkTurquoise));
        edit()->styleSetFore(SCE_C_NUMBER, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_C_WORD, StyleColor::color(StyleColor::Table::get()->Gold));
        edit()->styleSetFore(SCE_C_STRING, StyleColor::color(StyleColor::Table::get()->Magenta)); // 字符串
        edit()->styleSetFore(SCE_C_CHARACTER, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_C_UUID, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_C_PREPROCESSOR, StyleColor::color(StyleColor::Table::get()->MediumBlue)); // #
        edit()->styleSetFore(SCE_C_OPERATOR, edit()->styleFore(SCE_C_DEFAULT)); // 符号
        edit()->styleSetFore(SCE_C_IDENTIFIER, edit()->styleFore(SCE_C_DEFAULT));
        edit()->styleSetFore(SCE_C_STRINGEOL, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_C_VERBATIM, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_C_REGEX, StyleColor::color(StyleColor::Table::get()->Magenta));
        edit()->styleSetFore(SCE_C_COMMENTLINEDOC, StyleColor::color(StyleColor::Table::get()->DarkTurquoise)); // ///注释
        edit()->styleSetFore(SCE_C_WORD2, StyleColor::color(StyleColor::Table::get()->DarkMagenta)); // 1 一般关键字
        edit()->styleSetFore(SCE_C_COMMENTDOCKEYWORD, StyleColor::color(StyleColor::Table::get()->DeepSkyBlue));
        edit()->styleSetFore(SCE_C_COMMENTDOCKEYWORDERROR, StyleColor::color(StyleColor::Table::get()->DeepSkyBlue)); // /// @
        edit()->styleSetFore(SCE_C_GLOBALCLASS, StyleColor::color(StyleColor::Table::get()->Gold));
        edit()->styleSetFore(SCE_C_STRINGRAW, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_C_HASHQUOTEDSTRING, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_C_PREPROCESSORCOMMENT, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_C_PREPROCESSORCOMMENTDOC, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_C_USERLITERAL, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_C_TASKMARKER, StyleColor::color(StyleColor::Table::get()->Gainsboro));
        edit()->styleSetFore(SCE_C_ESCAPESEQUENCE, StyleColor::color(StyleColor::Table::get()->Gainsboro));
    }
}
