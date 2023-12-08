// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stylescipython.h"
#include "textedittabwidget/style/stylejsonfile.h"
#include "textedittabwidget/style/stylecolor.h"
#include "textedittabwidget/textedit.h"
#include "SciLexer.h"

StyleSciPython::StyleSciPython(TextEdit *parent)
    : StyleSci (parent)
{

}

QMap<int, QString> StyleSciPython::keyWords() const
{
    return StyleSci::keyWords();
}

void StyleSciPython::setStyle()
{
    StyleSci::setStyle();
}

void StyleSciPython::setLexer()
{
    StyleSci::setLexer();
}

int StyleSciPython::sectionEnd() const
{
    return SCE_P_FTRIPLEDOUBLE;
}

void StyleSciPython::setThemeColor(DGuiApplicationHelper::ColorType colorType)
{
    StyleSci::setThemeColor(colorType);

    QJsonObject tempObj;
    int tempFore;
    auto jsonFile = edit()->getStyleFile();
    if (colorType == DGuiApplicationHelper::DarkType) {
        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Comment).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_COMMENTLINE, tempFore);
        edit()->styleSetFore(SCE_P_COMMENTBLOCK, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Class).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_CLASSNAME, tempFore);
        edit()->styleSetFore(SCE_P_DEFNAME, tempFore);
        edit()->styleSetFore(SCE_P_TRIPLE, tempFore);
        edit()->styleSetFore(SCE_P_TRIPLEDOUBLE, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Variable).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_IDENTIFIER, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Number).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_NUMBER, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Keyword).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_WORD, tempFore);
        edit()->styleSetFore(SCE_P_WORD2, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->String).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_STRING, tempFore);
        edit()->styleSetFore(SCE_P_CHARACTER, tempFore);
        edit()->styleSetFore(SCE_P_STRINGEOL, tempFore);
        edit()->styleSetFore(SCE_P_DECORATOR, tempFore);
        edit()->styleSetFore(SCE_P_FSTRING, tempFore);
        edit()->styleSetFore(SCE_P_FCHARACTER, tempFore);
        edit()->styleSetFore(SCE_P_FTRIPLE, tempFore);
        edit()->styleSetFore(SCE_P_FTRIPLEDOUBLE, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Operators).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_OPERATOR, tempFore); // 符号
    } else { // TODO(mozart):same with dark,should use light theme.
        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Comment).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_COMMENTLINE, tempFore);
        edit()->styleSetFore(SCE_P_COMMENTBLOCK, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Class).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_CLASSNAME, tempFore);
        edit()->styleSetFore(SCE_P_DEFNAME, tempFore);
        edit()->styleSetFore(SCE_P_TRIPLE, tempFore);
        edit()->styleSetFore(SCE_P_TRIPLEDOUBLE, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Variable).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_IDENTIFIER, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Number).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_NUMBER, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Keyword).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_WORD, tempFore);
        edit()->styleSetFore(SCE_P_WORD2, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->String).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_STRING, tempFore);
        edit()->styleSetFore(SCE_P_CHARACTER, tempFore);
        edit()->styleSetFore(SCE_P_STRINGEOL, tempFore);
        edit()->styleSetFore(SCE_P_DECORATOR, tempFore);
        edit()->styleSetFore(SCE_P_FSTRING, tempFore);
        edit()->styleSetFore(SCE_P_FCHARACTER, tempFore);
        edit()->styleSetFore(SCE_P_FTRIPLE, tempFore);
        edit()->styleSetFore(SCE_P_FTRIPLEDOUBLE, tempFore);

        tempObj = jsonFile->value(StyleJsonFile::Key_1::get()->Operators).toObject();
        tempFore = StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16));
        edit()->styleSetFore(SCE_P_OPERATOR, tempFore); // 符号
    }
}
