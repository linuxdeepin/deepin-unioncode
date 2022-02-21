#include "stylescicpp.h"
#include "textedittabwidget/style/stylecolor.h"

#include "SciLexer.h"

StyleSciCpp::StyleSciCpp()
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
            "virtual void volatile wchar_t while xor xor_eq"},
        { 3, "a addindex addtogroup anchor arg attention author b "
            "brief bug c class code date def defgroup deprecated "
            "dontinclude e em endcode endhtmlonly endif "
            "endlatexonly endlink endverbatim enum example "
            "exception f$ f[ f] file fn hideinitializer "
            "htmlinclude htmlonly if image include ingroup "
            "internal invariant interface latexonly li line link "
            "mainpage name namespace nosubgrouping note overload "
            "p page par param post pre ref relates remarks return "
            "retval sa section see showinitializer since skip "
            "skipline struct subsection test throw todo typedef "
            "union until var verbatim verbinclude version warning "
            "weakgroup $ @ \\ & < > # { }"
        }
    };
}

void StyleSciCpp::setStyle(ScintillaEdit &edit)
{
    StyleSci::setStyle(edit);

    edit.styleSetFore(SCE_C_COMMENT, StyleColor::color(StyleColor::Table::get()->LightSlateBlue)); // #整行
    edit.styleSetFore(SCE_C_COMMENTLINE, StyleColor::color(StyleColor::Table::get()->DarkTurquoise)); // //注释
    edit.styleSetFore(SCE_C_COMMENTDOC, StyleColor::color(StyleColor::Table::get()->DarkTurquoise));
    edit.styleSetFore(SCE_C_NUMBER, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit.styleSetFore(SCE_C_WORD, StyleColor::color(StyleColor::Table::get()->Gold));
    edit.styleSetFore(SCE_C_STRING, StyleColor::color(StyleColor::Table::get()->Magenta)); // 字符串
    edit.styleSetFore(SCE_C_CHARACTER, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit.styleSetFore(SCE_C_UUID, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit.styleSetFore(SCE_C_PREPROCESSOR, StyleColor::color(StyleColor::Table::get()->MediumBlue)); // #
    edit.styleSetFore(SCE_C_OPERATOR, edit.styleFore(SCE_C_DEFAULT)); // 符号
    edit.styleSetFore(SCE_C_IDENTIFIER, edit.styleFore(SCE_C_DEFAULT));
    edit.styleSetFore(SCE_C_STRINGEOL, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit.styleSetFore(SCE_C_VERBATIM, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit.styleSetFore(SCE_C_REGEX, StyleColor::color(StyleColor::Table::get()->Magenta));
    edit.styleSetFore(SCE_C_COMMENTLINEDOC, StyleColor::color(StyleColor::Table::get()->DarkTurquoise)); // ///注释
    edit.styleSetFore(SCE_C_WORD2, StyleColor::color(StyleColor::Table::get()->Gold)); // 1 一般关键字
    edit.styleSetFore(SCE_C_COMMENTDOCKEYWORD, StyleColor::color(StyleColor::Table::get()->DeepSkyBlue));
    edit.styleSetFore(SCE_C_COMMENTDOCKEYWORDERROR, StyleColor::color(StyleColor::Table::get()->DeepSkyBlue)); // /// @
    edit.styleSetFore(SCE_C_GLOBALCLASS, StyleColor::color(StyleColor::Table::get()->Gold));
    edit.styleSetFore(SCE_C_STRINGRAW, StyleColor::color(StyleColor::Table::get()->Gainsboro));
    edit.styleSetFore(SCE_C_HASHQUOTEDSTRING, StyleColor::color(StyleColor::Table::get()->Gainsboro));
    edit.styleSetFore(SCE_C_PREPROCESSORCOMMENT, StyleColor::color(StyleColor::Table::get()->Gainsboro));
    edit.styleSetFore(SCE_C_PREPROCESSORCOMMENTDOC, StyleColor::color(StyleColor::Table::get()->Gainsboro));
    edit.styleSetFore(SCE_C_USERLITERAL, StyleColor::color(StyleColor::Table::get()->Gainsboro));
    edit.styleSetFore(SCE_C_TASKMARKER, StyleColor::color(StyleColor::Table::get()->Gainsboro));
    edit.styleSetFore(SCE_C_ESCAPESEQUENCE, StyleColor::color(StyleColor::Table::get()->Gainsboro));

    return;
}

void StyleSciCpp::setLexer(ScintillaEdit &edit)
{
    return StyleSci::setLexer(edit);
}

int StyleSciCpp::styleOffset() const
{
    return SCE_C_ESCAPESEQUENCE; //default style key end;
}
