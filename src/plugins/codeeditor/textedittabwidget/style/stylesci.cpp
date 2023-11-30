// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
        auto mergeKeywords = (codeKeyWords + fileKeyWords).values().join(" ").toLatin1();
        d->edit->setKeyWords(key, mergeKeywords);
    }

    return;
}

void StyleSci::setThemeColor(DGuiApplicationHelper::ColorType themeType)
{
    QString theme = StyleJsonFile::Theme::get()->Dark;
    if (themeType == DGuiApplicationHelper::LightType) {
        theme = StyleJsonFile::Theme::get()->Light;
    }

    auto fileJson = d->edit->getStyleFile();
    auto themes = fileJson->themes();
    if (themes.contains(theme)) {
        d->edit->styleResetDefault(); //clean all
        fileJson->setTheme(theme);

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
        for(int i = sectionStart(); i <= sectionEnd(); i++) {
            if (i == sectionStart())
                d->edit->styleSetFore(sectionStart(), self_foreground);
            d->edit->styleSetBack(i, self_background);
        }
        d->edit->styleHotSpot(sectionEnd() + 1);
        d->edit->styleSetUnderline(sectionEnd() + 1, true);
    }
}

void StyleSci::setStyle()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &StyleSci::setThemeColor);

    auto themeType = DGuiApplicationHelper::instance()->themeType();
    setThemeColor(themeType);

    d->edit->annotationSetStyleOffset(AnnotationInfo::Role::get()->Fatal.code);
    d->edit->annotationSetVisible(ANNOTATION_BOXED);

    d->edit->styleSetBack(AnnotationInfo::Role::get()->Note.code, StyleColor::color(StyleColor::Table::get()->Blue));
    d->edit->styleSetItalic(AnnotationInfo::Role::get()->Note.code, true);

    d->edit->styleSetFore(AnnotationInfo::Role::get()->Warning.code, StyleColor::color(StyleColor::Table::get()->Yellow));
    d->edit->styleSetBack(AnnotationInfo::Role::get()->Warning.code, StyleColor::color(StyleColor::Table::get()->Blue));
    d->edit->styleSetItalic(AnnotationInfo::Role::get()->Warning.code, true);

    d->edit->styleSetFore(AnnotationInfo::Role::get()->Error.code, StyleColor::color(StyleColor::Table::get()->Red));
    d->edit->styleSetBack(AnnotationInfo::Role::get()->Error.code, StyleColor::color(StyleColor::Table::get()->Blue));
    d->edit->styleSetItalic(AnnotationInfo::Role::get()->Error.code, true);

    d->edit->styleSetFore(AnnotationInfo::Role::get()->Fatal.code , StyleColor::color(StyleColor::Table::get()->Red));
    d->edit->styleSetBack(AnnotationInfo::Role::get()->Fatal.code, StyleColor::color(StyleColor::Table::get()->Blue));
    d->edit->styleSetItalic(AnnotationInfo::Role::get()->Fatal.code, true);

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
                            | 1 << MarkerNumber::RunningLineBackground
                            | 1 << MarkerNumber::CustomLineBackground);

    d->edit->markerDefine(MarkerNumber::Debug, SC_MARK_CIRCLE);
    d->edit->markerDefine(MarkerNumber::Running, SC_MARK_SHORTARROW);
    d->edit->markerDefine(MarkerNumber::RunningLineBackground, SC_MARK_BACKGROUND);
    d->edit->markerDefine(MarkerNumber::CustomLineBackground, SC_MARK_BACKGROUND);

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

int StyleSci::sectionEnd() const
{
    return 0;
}

int StyleSci::sectionStart() const
{
    return 0;
}
