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
#include "stylesci.h"
#include "stylecolor.h"
#include "stylejsonfile.h"
#include "textedittabwidget/textedit.h"

#include "common/common.h"
#include "ILexer.h"
#include "Lexilla.h"

#include <QJsonObject>
#include <QFileInfo>
#include <QLibrary>
#include <QDir>

#include <bitset>

QString lexillaFileName()
{
    return QString(LEXILLA_LIB) + LEXILLA_EXTENSION;
}

QString lexillaFilePath()
{
    return CustomPaths::global(CustomPaths::DependLibs)
            + QDir::separator() + lexillaFileName();
}

sptr_t createLexerFromLib(const char *LanguageID)
{
    QFileInfo info(lexillaFilePath());
    if (!info.exists()) {
        qCritical() << "Failed, can't found lexilla library: " << info.filePath();
        abort();
    }

    static QLibrary lexillaLibrary(info.filePath());
    if (!lexillaLibrary.isLoaded()) {
        if (!lexillaLibrary.load()) {
            qCritical() << "Failed, to loading lexilla library: "
                        << info.filePath()
                        << lexillaLibrary.errorString();
            abort();
        }
        qInfo() << "Successful, Loaded lexilla library:" << info.filePath()
                << "\nand lexilla library support language count:";

#ifdef QT_DEBUG
        int langIDCount = ((Lexilla::GetLexerCountFn)(lexillaLibrary.resolve(LEXILLA_GETLEXERCOUNT)))();
        QStringList sciSupportLangs;
        for (int i = 0; i < langIDCount; i++) {
            sciSupportLangs << ((Lexilla::LexerNameFromIDFn)(lexillaLibrary.resolve(LEXILLA_LEXERNAMEFROMID)))(i);
        }
        qInfo() << "scintilla support language: " << sciSupportLangs;
#endif
    }
    QFunctionPointer fn = lexillaLibrary.resolve(LEXILLA_CREATELEXER);
    if (!fn) {
        qCritical() << lexillaLibrary.errorString();
        abort();
    }
    void *lexer = ((Lexilla::CreateLexerFn)fn)(LanguageID);
    return sptr_t(lexer);
}

class StyleSciPrivate
{
    friend class StyleSci;
    TextEdit *edit;
};

void StyleSci::setKeyWords()
{
    auto tokens = support_file::Language::tokenWords(d->edit->supportLanguage());
    auto codeKeyWordsMap = this->keyWords();

    for (auto key : codeKeyWordsMap.keys()) {
        auto codeKeyWords = codeKeyWordsMap[key].split(" ").toSet();
        auto fileKeyWords = tokens.value(key).split(" ").toSet();
        auto mergeKeywords = (codeKeyWords + fileKeyWords).toList().join(" ").toLatin1();
        d->edit->setKeyWords(key, mergeKeywords);
    }

    return;
}

void StyleSci::setStyle()
{
    auto fileJson = d->edit->getStyleFile();

    auto themes = fileJson->themes();
    if (!themes.isEmpty()) {
        d->edit->styleResetDefault(); //clean all
        fileJson->setTheme(themes.first());

        auto selfObj = fileJson->value(StyleJsonFile::Key_1::get()->Self).toObject();
        auto self_foreground = selfObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16);
        auto self_background = selfObj.value(StyleJsonFile::Key_2::get()->Background).toString().toInt(nullptr, 16);
        auto self_cursor = selfObj.value(StyleJsonFile::Key_2::get()->Cursor).toString().toInt(nullptr, 16);
        d->edit->styleSetFore(STYLE_DEFAULT, self_foreground);
        d->edit->styleSetBack(STYLE_DEFAULT, self_background);
        d->edit->setCaretFore(self_cursor);
        qInfo() << "Editor self jsonObject:" << selfObj;
        qInfo() << "Editor self setting Style fore: " << hex << self_foreground;
        qInfo() << "Editor self setting Style back: " << hex << self_background;
        qInfo() << "Editor self setting Style cursor: " << hex << self_cursor;
        // auto self_fontSize = selfObj.value(key_2.FontSize).toInt();
        // d->edit->styleSetSize(STYLE_DEFAULT, self_fontSize);
        for(int i = 0; i <= styleOffset(); i++) {
            if (i == 0)
                d->edit->styleSetFore(0, self_foreground);
            d->edit->styleSetBack(i, self_background);
        }
        d->edit->styleHotSpot(styleOffset() + 1);
        d->edit->styleSetUnderline(styleOffset() + 1, true);
    }
    return;
}

void StyleSci::setMargin()
{
    d->edit->setMargins(SC_MAX_MARGIN);

    d->edit->setMarginSensitiveN(Margin::LineNumber, SCN_MARGINCLICK);
    d->edit->setMarginWidthN(Margin::LineNumber, d->edit->textWidth(STYLE_LINENUMBER ,"999999"));
    d->edit->setMarginBackN(Margin::LineNumber, d->edit->styleBack(STYLE_DEFAULT));

    //runtime margin
    d->edit->setMarginSensitiveN(Margin::Runtime, SCN_MARGINCLICK);
    d->edit->setMarginWidthN(Margin::Runtime, 16);
    d->edit->setMarginTypeN(Margin::Runtime, SC_MARGIN_SYMBOL);
    d->edit->setMarginMaskN(Margin::Runtime, 1 << MarkerNumber::Debug
                        | 1 << MarkerNumber::Running
                        | 1<< MarkerNumber::RunningLineBackground);

    d->edit->markerDefine(MarkerNumber::Debug, SC_MARK_CIRCLE);
    d->edit->markerDefine(MarkerNumber::Running, SC_MARK_SHORTARROW);
    d->edit->markerDefine(MarkerNumber::RunningLineBackground, SC_MARK_BACKGROUND);

    d->edit->markerSetFore(MarkerNumber::Debug, StyleColor::color(StyleColor::Table::get()->FireBrick));
    d->edit->markerSetBack(MarkerNumber::Debug, StyleColor::color(StyleColor::Table::get()->FireBrick));

    d->edit->markerSetFore(MarkerNumber::Running, StyleColor::color(StyleColor::Table::get()->YellowGreen));
    d->edit->markerSetBack(MarkerNumber::Running, StyleColor::color(StyleColor::Table::get()->YellowGreen));

    d->edit->markerSetFore(MarkerNumber::RunningLineBackground, StyleColor::color(StyleColor::Table::get()->YellowGreen));
    d->edit->markerSetBack(MarkerNumber::RunningLineBackground, StyleColor::color(StyleColor::Table::get()->YellowGreen));
    d->edit->markerSetAlpha(MarkerNumber::RunningLineBackground, 0x55);
}

void StyleSci::setLexer()
{
    if (!d->edit->lexer()) {
        //set token splitter
        d->edit->setILexer(createLexerFromLib(d->edit->supportLanguage().toLatin1()));
        if (!d->edit->lexer()) {
            qCritical() << "Failed, can't create and load sci lexer";
        }
    }
}

StyleSci::StyleSci(TextEdit *parent)
    : QObject (parent)
    , d (new StyleSciPrivate())
{
    d->edit = parent;
}

StyleSci::~StyleSci()
{
    if (d) {
        delete d;
    }
}

TextEdit *StyleSci::edit()
{
    return d->edit;
}

QMap<int, QString> StyleSci::keyWords() const
{
    QMap<int, QString> result;
    for(int i = 0; i <= KEYWORDSET_MAX; i++) {
        result[i] = "";
    }
    return result;
}

int StyleSci::styleOffset() const
{
    return 0;
}
