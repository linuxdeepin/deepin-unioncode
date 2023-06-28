// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stylescicmake.h"
#include "textedittabwidget/style/stylecolor.h"
#include "textedittabwidget/textedit.h"
#include "SciLexer.h"

StyleSciCmake::StyleSciCmake(TextEdit *parent)
    : StyleSci (parent)
{

}

QMap<int, QString> StyleSciCmake::keyWords() const
{
    return StyleSci::keyWords();
}

void StyleSciCmake::setStyle()
{
    StyleSci::setStyle();
    edit()->styleSetFore(SCE_CMAKE_COMMENT, StyleColor::color(StyleColor::Table::get()->SkyBlue));
    edit()->styleSetFore(SCE_CMAKE_STRINGDQ, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit()->styleSetFore(SCE_CMAKE_STRINGLQ, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit()->styleSetFore(SCE_CMAKE_STRINGRQ, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit()->styleSetFore(SCE_CMAKE_COMMANDS, StyleColor::color(StyleColor::Table::get()->PaleGoldenRod));
    edit()->styleSetFore(SCE_CMAKE_PARAMETERS, StyleColor::color(StyleColor::Table::get()->Orange));
    edit()->styleSetFore(SCE_CMAKE_VARIABLE, StyleColor::color(StyleColor::Table::get()->Orange));
    edit()->styleSetFore(SCE_CMAKE_USERDEFINED, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit()->styleSetFore(SCE_CMAKE_WHILEDEF, StyleColor::color(StyleColor::Table::get()->Gold));
    edit()->styleSetFore(SCE_CMAKE_FOREACHDEF, StyleColor::color(StyleColor::Table::get()->Gold));
    edit()->styleSetFore(SCE_CMAKE_IFDEFINEDEF, StyleColor::color(StyleColor::Table::get()->Gold));
    edit()->styleSetFore(SCE_CMAKE_MACRODEF, StyleColor::color(StyleColor::Table::get()->Gold));
    edit()->styleSetFore(SCE_CMAKE_STRINGVAR, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit()->styleSetFore(SCE_CMAKE_NUMBER, StyleColor::color(StyleColor::Table::get()->Magenta));
}

void StyleSciCmake::setLexer()
{
    StyleSci::setLexer();
}

int StyleSciCmake::sectionEnd() const
{
    return SCE_CMAKE_NUMBER;
}
