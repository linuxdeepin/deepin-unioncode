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

    auto jsonFile = edit()->getStyleFile();
    if (jsonFile->setTheme(StyleJsonFile::Theme::get()->Dark)) {
        QJsonObject tempObj;
        int tempFore;
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

void StyleSciPython::setLexer()
{
    StyleSci::setLexer();
}

int StyleSciPython::sectionEnd() const
{
    return SCE_P_FTRIPLEDOUBLE;
}
