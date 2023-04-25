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
