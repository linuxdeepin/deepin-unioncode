#include "stylesci.h"
#include "stylekeeper.h"
#include "stylecolor.h"
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
    void *lexCpp = ((Lexilla::CreateLexerFn)fn)(LanguageID);
    return sptr_t(lexCpp);
}

void StyleSci::setKeyWords(ScintillaEdit &edit)
{
    auto language = StyleKeeper::key(this);
    auto tokens = support_file::Language::tokenWords(language);
    auto codeKeyWordsMap = this->keyWords();

    for (auto key : codeKeyWordsMap.keys()) {
        auto codeKeyWords = codeKeyWordsMap[key].split(" ").toSet();
        auto fileKeyWords = tokens.value(key).split(" ").toSet();
        auto mergeKeywords = (codeKeyWords + fileKeyWords).toList().join(" ").toLatin1();
        edit.setKeyWords(key, mergeKeywords);
    }

    return;
}

void StyleSci::setStyle(ScintillaEdit &edit)
{
    auto fileJson = StyleKeeper::fileJson(StyleKeeper::key(this));
    auto themes = fileJson->themes();
    if (!themes.isEmpty()) {
        edit.styleResetDefault(); //clean all
        fileJson->setTheme(themes.first());

        auto selfObj = fileJson->value(StyleJsonFile::Key_1::get()->Self).toObject();
        auto self_foreground = selfObj.value(StyleJsonFile::Key_2::get()->Foreground).toString().toInt(nullptr, 16);
        edit.styleSetFore(STYLE_DEFAULT, self_foreground);
        auto self_background = selfObj.value(StyleJsonFile::Key_2::get()->Background).toString().toInt(nullptr, 16);
        edit.styleSetBack(STYLE_DEFAULT, self_background);
        // auto self_fontSize = selfObj.value(key_2.FontSize).toInt();
        // edit.styleSetSize(STYLE_DEFAULT, self_fontSize);
        auto self_cursor = selfObj.value(StyleJsonFile::Key_2::get()->Cursor).toString().toInt(nullptr, 16);
        edit.setCaretFore(self_cursor);

        for(int i = 0; i <= styleOffset(); i++) {
            if (i == 0)
                edit.styleSetFore(0, self_foreground);
            edit.styleSetBack(i, self_background);
        }
    }
    return;
}

void StyleSci::setMargin(ScintillaEdit &edit)
{
    edit.setMargins(SC_MAX_MARGIN);

    edit.setMarginSensitiveN(Margin::LineNumber, SCN_MARGINCLICK);
    edit.setMarginWidthN(Margin::LineNumber, edit.textWidth(STYLE_LINENUMBER ,"999999"));
    edit.setMarginBackN(Margin::LineNumber, edit.styleBack(STYLE_DEFAULT));

    //runtime margin
    edit.setMarginSensitiveN(Margin::Runtime, SCN_MARGINCLICK);
    edit.setMarginWidthN(Margin::Runtime, 16);
    edit.setMarginTypeN(Margin::Runtime, SC_MARGIN_SYMBOL);
    edit.setMarginMaskN(Margin::Runtime, 1 << MarkerNumber::Debug
                        | 1 << MarkerNumber::Running
                        | 1<< MarkerNumber::RunningLineBackground);

    edit.markerDefine(MarkerNumber::Debug, SC_MARK_CIRCLE);
    edit.markerDefine(MarkerNumber::Running, SC_MARK_SHORTARROW);
    edit.markerDefine(MarkerNumber::RunningLineBackground, SC_MARK_BACKGROUND);

    edit.markerSetFore(MarkerNumber::Debug, StyleColor::color(StyleColor::Table::get()->FireBrick));
    edit.markerSetBack(MarkerNumber::Debug, StyleColor::color(StyleColor::Table::get()->FireBrick));

    edit.markerSetFore(MarkerNumber::Running, StyleColor::color(StyleColor::Table::get()->YellowGreen));
    edit.markerSetBack(MarkerNumber::Running, StyleColor::color(StyleColor::Table::get()->YellowGreen));

    edit.markerSetFore(MarkerNumber::RunningLineBackground, StyleColor::color(StyleColor::Table::get()->YellowGreen));
    edit.markerSetBack(MarkerNumber::RunningLineBackground, StyleColor::color(StyleColor::Table::get()->YellowGreen));
    edit.markerSetAlpha(MarkerNumber::RunningLineBackground, 0x55);
}

void StyleSci::setLexer(ScintillaEdit &edit)
{
    if (!edit.lexer()) {
        //set token splitter
        edit.setILexer(createLexerFromLib(StyleKeeper::key(this).toLatin1()));
        if (!edit.lexer()) {
            qCritical() << "Failed, can't create and load sci lexer";
        }
    }
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
