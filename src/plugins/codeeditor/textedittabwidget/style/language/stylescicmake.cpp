#include "stylescicmake.h"
#include "textedittabwidget/style/stylecolor.h"

#include "SciLexer.h"

StyleSciCmake::StyleSciCmake()
{

}

QMap<int, QString> StyleSciCmake::keyWords() const
{
    return StyleSci::keyWords();
}

void StyleSciCmake::setStyle(ScintillaEdit &edit)
{
    StyleSci::setStyle(edit);
    edit.styleSetFore(SCE_CMAKE_COMMENT, StyleColor::color(StyleColor::Table::get()->SkyBlue));
    edit.styleSetFore(SCE_CMAKE_STRINGDQ, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit.styleSetFore(SCE_CMAKE_STRINGLQ, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit.styleSetFore(SCE_CMAKE_STRINGRQ, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit.styleSetFore(SCE_CMAKE_COMMANDS, StyleColor::color(StyleColor::Table::get()->PaleGoldenRod));
    edit.styleSetFore(SCE_CMAKE_PARAMETERS, StyleColor::color(StyleColor::Table::get()->Orange));
    edit.styleSetFore(SCE_CMAKE_VARIABLE, StyleColor::color(StyleColor::Table::get()->Orange));
    edit.styleSetFore(SCE_CMAKE_USERDEFINED, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit.styleSetFore(SCE_CMAKE_WHILEDEF, StyleColor::color(StyleColor::Table::get()->Gold));
    edit.styleSetFore(SCE_CMAKE_FOREACHDEF, StyleColor::color(StyleColor::Table::get()->Gold));
    edit.styleSetFore(SCE_CMAKE_IFDEFINEDEF, StyleColor::color(StyleColor::Table::get()->Gold));
    edit.styleSetFore(SCE_CMAKE_MACRODEF, StyleColor::color(StyleColor::Table::get()->Gold));
    edit.styleSetFore(SCE_CMAKE_STRINGVAR, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit.styleSetFore(SCE_CMAKE_NUMBER, StyleColor::color(StyleColor::Table::get()->Magenta));
}

void StyleSciCmake::setLexer(ScintillaEdit &edit)
{
    return StyleSci::setLexer(edit);
}

int StyleSciCmake::styleOffset() const
{
    return SCE_CMAKE_NUMBER;
}
