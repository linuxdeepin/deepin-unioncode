// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stylescijs.h"
#include "textedittabwidget/style/stylejsonfile.h"
#include "textedittabwidget/style/stylecolor.h"
#include "textedittabwidget/textedit.h"
#include "SciLexer.h"

StyleSciJS::StyleSciJS(TextEdit *parent)
    : StyleSci(parent)
{
}

QMap<int, QString> StyleSciJS::keyWords() const
{
    return StyleSci::keyWords();
}

void StyleSciJS::setStyle()
{
    StyleSci::setStyle();

    auto jsonFile = edit()->getStyleFile();
    if (jsonFile->setTheme(StyleJsonFile::Theme::get()->Dark)) {
        auto highLightColor = [&](const QString &key) -> int {
            QJsonObject tempObj = jsonFile->value(key).toObject();
            return StyleColor::color(tempObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toUInt(nullptr, 16));
        };
        // comment.
        int color = highLightColor(StyleJsonFile::Key_1::get()->Comment);
        edit()->styleSetFore(SCE_HJ_COMMENTLINE, color);
        edit()->styleSetFore(SCE_HJ_COMMENT, color);
        edit()->styleSetFore(SCE_HJ_COMMENTDOC, color);

        // number.
        color = highLightColor(StyleJsonFile::Key_1::get()->Number);
        edit()->styleSetFore(SCE_HJ_NUMBER, color);

        // keyword.
        color = highLightColor(StyleJsonFile::Key_1::get()->Keyword);
        edit()->styleSetFore(SCE_HJ_KEYWORD, color);

        // string.
        color = highLightColor(StyleJsonFile::Key_1::get()->String);
        edit()->styleSetFore(SCE_HJ_DOUBLESTRING, color);
        edit()->styleSetFore(SCE_HJ_DOUBLESTRING, color);

        // word.
        color = highLightColor(StyleJsonFile::Key_1::get()->Text);
        edit()->styleSetFore(SCE_HJ_WORD, color);
    }
}

void StyleSciJS::setLexer()
{
    StyleSci::setLexer();
}

int StyleSciJS::sectionEnd() const
{
    return SCE_HJ_REGEX;
}

int StyleSciJS::sectionStart() const
{
    return SCE_HJ_START;
}
