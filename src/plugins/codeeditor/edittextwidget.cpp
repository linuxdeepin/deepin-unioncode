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
#include "edittextwidget.h"
#include "edittextwidgetstyle.h"
#include "filelangdatabase.h"
#include "sendevents.h"
#include "Document.h"
#include "Lexilla.h"
#include "config.h" //cmake build generate
#include "SciLexer.h"
#include "common/util/processutil.h"
#include "common/dialog/contextdialog.h"
#include "common/util/custompaths.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QDir>
#include <QDebug>
#include <QLibrary>
#include <QApplication>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QRegularExpression>

#include <iostream>

Sci_Position getSciPosition(sptr_t doc, const lsp::Position &pos)
{
    auto docTemp = (Scintilla::Internal::Document*)(doc);
    return docTemp->GetRelativePosition(docTemp->LineStart(pos.line), pos.character);
}

lsp::Position getLspPosition(sptr_t doc, sptr_t sciPosition)
{
    auto docTemp = (Scintilla::Internal::Document*)(doc);
    int line = docTemp->LineFromPosition(sciPosition);
    Sci_Position lineChStartPos = getSciPosition(doc, lsp::Position{line, 0});
    return lsp::Position{line, (int)(sciPosition - lineChStartPos)};
}

int getLspCharacter(sptr_t doc, sptr_t sciPosition)
{
    return getLspPosition(doc, sciPosition).character;
}

QString lexillaFileName()
{
    return QString(LEXILLA_LIB) + LEXILLA_EXTENSION;
}

QString lexillaFilePath()
{
    if (CustomPaths::installed())
        return QString(LEXILLA_INSTALL_PATH) + QDir::separator() + lexillaFileName();
    else
        return QString(LEXILLA_BUILD_PATH)  + QDir::separator() + lexillaFileName();
}

QString languageSupportFilePath()
{
    if (CustomPaths::installed())
        return QString(SUPPORT_INSTALL_PATH) + QDir::separator() + "language.support";
    else
        return QString(SUPPORT_BUILD_PATH) + QDir::separator() + "language.support";
}

QString languageServer(const QString &filePath,
                       QString *server = nullptr,
                       QStringList *serverArguments = nullptr,
                       QStringList *tokenWords = nullptr,
                       QStringList *suffixs = nullptr,
                       QStringList *bases = nullptr)
{
    QFile file(languageSupportFilePath());
    QJsonDocument jsonDoc;
    if (file.open(QFile::ReadOnly)) {
        auto readall = file.readAll();
        jsonDoc = QJsonDocument::fromJson(readall);
        file.close();
    }

    if (jsonDoc.isEmpty()) {
        ContextDialog::ok(EditTextWidget::tr("The format of the language configuration file is incorrect or damaged. "
                                             "Check that the file is released correctly. "
                                             "If it cannot be solved, reinstall the software to solve the problem"));
        qCritical() << QString("Failed, %0 jsonDoc is Empty.").arg(languageSupportFilePath());
        abort();
    }

    QJsonObject jsonObj = jsonDoc.object();
    qInfo() << "configure file support language:" << jsonObj.keys();
    for (auto val : jsonObj.keys()) {
        auto langObjChild = jsonObj.value(val).toObject();
        QFileInfo info(filePath);
        QString serverProgram = langObjChild.value("server").toString();
        QJsonArray suffixArray = langObjChild.value("suffix").toArray();
        QJsonArray baseArray = langObjChild.value("base").toArray();
        QJsonArray initArguments = langObjChild.value("serverArguments").toArray();
        QJsonArray tokenWordSet = langObjChild.value("tokenWords").toArray();

        bool isContainsSuffix = false;
        QStringList temp;
        for (auto suffix : suffixArray) {
            if (info.fileName().endsWith(suffix.toString()))
                isContainsSuffix = true;

            if (info.suffix() == suffix.toString())
                isContainsSuffix = true;

            temp.append(suffix.toString());
        }
        if (isContainsSuffix && suffixs)
            *suffixs = temp;

        temp.clear();
        bool isContainsBase = false;
        for (auto base : baseArray) {

            if (info.fileName() == base.toString()
                    || info.fileName().toLower() == base.toString().toLower()) {
                isContainsBase = true;
            }

            temp.append(base.toString());
        }
        if (isContainsBase && bases)
            *bases = temp;

        if (serverArguments)
            for (auto arg: initArguments) {
                serverArguments->append(arg.toString());
            }

        if (tokenWords)
            for (auto set : tokenWordSet) {
                tokenWords->append(set.toString());
            }

        if (server)
            *server = serverProgram;

        if (isContainsBase || isContainsSuffix)
            return val;
    }

    return "";
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

class EditTextWidgetPrivate
{
    friend class EditTextWidget;

    inline int scintillaColor(const QColor &col)
    {
        return (col.blue() << 16) | (col.green() << 8) | col.red();
    }

    QPoint hoverPos = {-1, -1};
    QString file = "";
    lsp::Client *client = nullptr;
    EditTextWidgetStyle style;
    QString currentLanguageId{};
    bool lspIndicStyleLoaded = false;
    QStringList tokenTypesCache{};
    QStringList tokenModifiersCache{};
    lsp::Position editInsertPostion{-1, -1};
    int editInsertCount = 0;
};

EditTextWidget::EditTextWidget(QWidget *parent)
    : ScintillaEdit (parent)
    , d(new EditTextWidgetPrivate)
{
    setMouseDwellTime(1000);
    setDefaultStyle();

    QObject::connect(this, &EditTextWidget::marginClicked, this, &EditTextWidget::debugMarginClieced);

    QObject::connect(this, &ScintillaEditBase::horizontalScrolled, this, [=](int value)
    {qInfo() << "horizontalScrolled" ;});
    QObject::connect(this, &ScintillaEditBase::verticalScrolled, this, [=](int value)
    {qInfo() << "verticalScrolled" ;});
    QObject::connect(this, &ScintillaEditBase::horizontalRangeChanged, this, [=](int max, int page)
    {qInfo() << "horizontalScrolled" ;});
    QObject::connect(this, &ScintillaEditBase::verticalRangeChanged, this, [=](int max, int page)
    {qInfo() << "verticalRangeChanged" ;});
    //    QObject::connect(this, &ScintillaEditBase::notifyChange, this, [=]()
    //    {qInfo() << "notifyChange" ;});
    QObject::connect(this, &ScintillaEditBase::linesAdded, this, [=](Scintilla::Position linesAdded)
    {qInfo() << "linesAdded" ;});
    QObject::connect(this, &ScintillaEditBase::aboutToCopy, this, [=](QMimeData *data)
    {qInfo() << "aboutToCopy" ;});
    QObject::connect(this, &ScintillaEditBase::styleNeeded, this, [=](Scintilla::Position position)
    {qInfo() << "styleNeeded" ;});

    // Scintilla Notifications
    //    QObject::connect(this, &ScintillaEditBase::charAdded, this, [=](int ch)
    //    {qInfo() << "charAdded" ;});
    QObject::connect(this, &ScintillaEditBase::savePointChanged, this, [=](bool dirty)
    {qInfo() << "savePointChanged" ;});
    QObject::connect(this, &ScintillaEditBase::modifyAttemptReadOnly, this, [=]()
    {qInfo() << "modifyAttemptReadOnly" ;});
    QObject::connect(this, &ScintillaEditBase::key, this, [=](int key)
    {qInfo() << "key" ;});
    QObject::connect(this, &ScintillaEditBase::doubleClick, this, [=](Scintilla::Position position, Scintilla::Position line)
    {qInfo() <<"doubleClick" ;});
    //    QObject::connect(this, &ScintillaEditBase::updateUi, this, [=](Scintilla::Update updated)
    //    {qInfo() << "updateUi" <<  QString().sprintf("%04x", updated); });
    QObject::connect(this, &ScintillaEditBase::macroRecord, this, [=](Scintilla::Message message, Scintilla::uptr_t wParam, Scintilla::sptr_t lParam)
    {qInfo()<< "macroRecord" ;});
    QObject::connect(this, &ScintillaEditBase::marginClicked, this, [=](Scintilla::Position position, Scintilla::KeyMod modifiers, int margin)
    {qInfo()<< "marginClicked" ;});
    QObject::connect(this, &ScintillaEditBase::textAreaClicked, this, [=](Scintilla::Position line, int modifiers)
    {qInfo()<< "textAreaClicked" ;});
    QObject::connect(this, &ScintillaEditBase::needShown, this, [=](Scintilla::Position position, Scintilla::Position length)
    {qInfo()<< "needShown" ;});
    //    QObject::connect(this, &ScintillaEditBase::painted, this, [=]()
    //    {qInfo()<< "painted" ;});
    QObject::connect(this, &ScintillaEditBase::userListSelection, this, [=]()
    {qInfo()<< "userListSelection" ;});// Wants some args.
    QObject::connect(this, &ScintillaEditBase::uriDropped, this, [=](const QString &uri)
    {qInfo()<< "uriDropped" ;});
    QObject::connect(this, &ScintillaEditBase::zoom, this, [=](int zoom)
    {qInfo()<< "zoom" ;});
    QObject::connect(this, &ScintillaEditBase::hotSpotClick, this, [=](Scintilla::Position position, Scintilla::KeyMod modifiers)
    {qInfo()<< "hotSpotClick" ;});
    QObject::connect(this, &ScintillaEditBase::hotSpotDoubleClick, this, [=](Scintilla::Position position, Scintilla::KeyMod modifiers)
    {qInfo()<< "hotSpotDoubleClick" ;});
    QObject::connect(this, &ScintillaEditBase::callTipClick, this, [=]()
    {qInfo()<< "callTipClick" ;});
    QObject::connect(this, &ScintillaEditBase::autoCompleteSelection, this, [=](Scintilla::Position position, const QString &text)
    {qInfo()<< "autoCompleteSelection";});
    QObject::connect(this, &ScintillaEditBase::autoCompleteCancelled, this, [=]()
    {qInfo()<< "autoCompleteCancelled";});
    QObject::connect(this, &ScintillaEditBase::focusChanged, this, [=](bool focused)
    {qInfo()<< "focusChanged";});

    // Base notifications for compatibility with other Scintilla implementations
    //    QObject::connect(this, &ScintillaEditBase::notify, this, &EditTextWidget::sciNotify);

    //    QObject::connect(this, &ScintillaEditBase::command, this, [=](Scintilla::uptr_t wParam, Scintilla::sptr_t lParam)
    //    {qInfo()<< "command" ;});

    // GUI event notifications needed under Qt
    QObject::connect(this, &ScintillaEditBase::buttonPressed, this, [=](QMouseEvent *event)
    {qInfo()<< "buttonPressed" ;});
    QObject::connect(this, &ScintillaEditBase::buttonReleased, this, [=](QMouseEvent *event)
    {qInfo()<< "buttonReleased" ;});
    //    QObject::connect(this, &ScintillaEditBase::keyPressed, this, [=](QKeyEvent *event)
    //    {qInfo()<< "keyPressed" ;});
    QObject::connect(this, &ScintillaEditBase::resized, this, [=]()
    {qInfo()<< "resized" ;});

    //    //
    //    //	Handle autocompletion start
    //    //
    //    connect(this, &LspScintillaEdit::lspCompletion, [=, &autoCompletion](const Scintilla::LspCompletionList &cl) {
    //        autoCompletion = cl;
    //        if (cl.items.size() == 0)
    //        {
    //            autoCCancel();
    //            return;
    //        }
    //        QStringList items;
    //        for (auto i : cl.items)
    //            items.append(QString::fromStdString(i.label).simplified());
    //        autoCSetSeparator('\t');
    //        autoCShow(0, items.join("\t").toUtf8().constData());
    //    });
    //    //
    //    //	Handle autocompletion apply
    //    //
    //    connect(this, &ScintillaEdit::autoCompleteSelection,
    //            [=, &autoCompletion](int position, const QString &text, QChar fillupChar, int listCompletionMethod) {
    //        Q_UNUSED(position)
    //        Q_UNUSED(text)
    //        Q_UNUSED(fillupChar)
    //        Q_UNUSED(listCompletionMethod)
    //        // Prendo l'indice selezionato e cancello la lista
    //        const int curIdx = autoCCurrent();
    //        if (curIdx < 0 || curIdx >= autoCompletion.items.size())
    //            return;
    //        autoCCancel();
    //        // Eseguo l'autocompletamento
    //        const auto &textEdit = autoCompletion.items.at(curIdx).textEdit;
    //        Scintilla::LspScintillaDoc doc(docPointer());
    //        Sci_Position p_start, p_end;
    //        if (!Scintilla::lspConv::convertRange(doc, textEdit.range, p_start, p_end))
    //            return;
    //        setSel(p_start, p_end);
    //        replaceSel(textEdit.newText.c_str());
    //    });
    //    //
    //    //	Handle signature help
    //    //
    //    connect(this, &LspScintillaEdit::lspSignatureHelp, [=](const Scintilla::LspSignatureHelp &help) {
    //        const std::size_t sz = help.signatures.size();
    //        if (!hasFocus() || (sz == 0))
    //            return;
    //        QByteArray text;
    //        Sci_Position hlt_start = 0, hlt_end = 0;
    //        if (sz == 1)
    //            text = QByteArray::fromStdString(help.signatures[0].label);
    //        else for (int i = 0; i < sz; i++)
    //        {
    //            const int idx = (help.activeSignature + i) % sz;
    //            if (!text.isEmpty()) text.append('\n');
    //            text.append("\001[");
    //            text.append(QByte
    //            text.append("]\002 ");
    //            if (idx == help.activeSignature)
    //            {
    //                const auto &par = help.signatures[idx].parameters[help.activeParameter];
    //                hlt_start = text.size() + par.label_start;
    //                hlt_end = hlt_start + (par.label_end - par.label_start);
    //            }
    //            text.append(help.signatures[idx].label.c_str());
    //        }
    //        callTipShow(currentPos(), text.constData());
    //        callTipSetHlt(hlt_start, hlt_end);
    //    });
    //
}

EditTextWidget::~EditTextWidget()
{
    if (d) {
        if (d->client) {
            d->client->shutdownRequest();
            d->client->waitForBytesWritten();
            d->client->kill();
            d->client->waitForFinished();
            delete d->client;
        }
        delete d;
    }
}

QString EditTextWidget::currentFile()
{
    return d->file;
}

void EditTextWidget::setCurrentFile(const QString &filePath, const QString &workspaceFolder)
{
    if (d->file == filePath) {
        return;
    }
    else {
        d->file = filePath;
    }
    QString text;
    QFile file(filePath);
    if (file.open(QFile::OpenModeFlag::ReadOnly)) {
        text = file.readAll();
        file.close();
    }
    setText(text.toUtf8());

    QString serverProgram;
    QStringList serverProgramOptions;
    QStringList tokenWords;
    QString languageID = languageServer(filePath, &serverProgram, &serverProgramOptions, &tokenWords);
    if (!lexer()) {
        setILexer(createLexerFromLib(languageID.toLatin1())); //set token splitter
        d->currentLanguageId = languageID;
        for (int i = 0; i < tokenWords.size(); i++) {
            if (i < KEYWORDSET_MAX)
                /* SCI_SETKEYWORDS(int keyWordSet, const char *keyWords)
                 * more see link https://www.scintilla.org/ScintillaDoc.html#SCI_SETKEYWORDS */
                setKeyWords(i, tokenWords.at(i).toLatin1()); //setting IDE self default token
        }
        QStringList themes = d->style.styleThemes(languageID);
        QList<int> sciKeys = d->style.sciKeys();

        if(themes.isEmpty()) {
            return;
        }

        // set self background
        styleSetBack(STYLE_DEFAULT, d->scintillaColor(QColor(d->style.backgroundSelf(languageID, themes.first()))));
        // set cursor foreground
        setCaretFore(d->scintillaColor(QColor(d->style.cursor(languageID, themes.first()))));

        // set language keys style
        foreach (auto sciKey, sciKeys) {
            //need user cache setting file set theme
            if (themes.isEmpty())
                break;
            styleSetFore(sciKey, d->scintillaColor(QColor(d->style.foreground(languageID, themes.first(), d->style.styleValue(sciKey)))));
            styleSetBack(sciKey, d->scintillaColor(QColor(d->style.background(languageID, themes.first(), d->style.styleValue(sciKey)))));
            // styleSetFont(sciKey,) 预留
        }
    }

    bool checkVersionOk = false;
    // exists language server
    if (!serverProgram.isEmpty() || ProcessUtil::exists(serverProgram)) {
        // clang version lower 7
        if (serverProgram == "clangd") {
            QRegularExpression regExp("[0-9]*\\.[0-9]*\\.[0-9]*");
            auto versionMatchs = regExp.match(ProcessUtil::version(serverProgram)).capturedTexts();
            for (auto versionMatch : versionMatchs){
                QStringList versions = versionMatch.split(".");
                if (versions.size() == 3) {
                    auto major =  versions[0];
                    if (major.toInt() > 7) { //版本小于7
                        checkVersionOk = true;
                    }
                }
            }

            if (!checkVersionOk) {
                ContextDialog::ok(EditTextWidget::tr("clangd lower verion: 7, Does not meet the current operating environment"));
                return;
            }
        }

        if (!d->lspIndicStyleLoaded)
            d->lspIndicStyleLoaded  = setLspIndicStyle(languageID);

        if (!d->client)
            d->client = new lsp::Client();

        //bind signals to file diagnostics
        QObject::connect(d->client, QOverload<const lsp::Diagnostics &>::of(&lsp::Client::notification),
                         this, &EditTextWidget::publishDiagnostics, Qt::UniqueConnection);

        QObject::connect(d->client, QOverload<const QList<lsp::Data>&>::of(&lsp::Client::requestResult),
                         this, &EditTextWidget::tokenFullResult, Qt::UniqueConnection);

        QObject::connect(d->client, QOverload<const lsp::SemanticTokensProvider&>::of(&lsp::Client::requestResult),
                         this, &EditTextWidget::tokenDefinitionsSave, Qt::UniqueConnection);

        QObject::connect(d->client, QOverload<const lsp::Hover&>::of(&lsp::Client::requestResult),
                         this, &EditTextWidget::hoverMessage, Qt::UniqueConnection);

        QObject::connect(d->client, QOverload<const lsp::CompletionProvider&>::of(&lsp::Client::requestResult),
                         this, &EditTextWidget::completionsSave, Qt::UniqueConnection);

        d->client->setProgram(serverProgram);
        d->client->setArguments(serverProgramOptions);
        d->client->start();
        d->client->initRequest(workspaceFolder);
        d->client->openRequest(filePath);
        d->client->docSemanticTokensFull(filePath);

        QObject::connect(this, &ScintillaEditBase::modified, this, &EditTextWidget::sciModified, Qt::UniqueConnection);
        QObject::connect(this, &EditTextWidget::dwellStart, this, &EditTextWidget::dwellStartNotify, Qt::UniqueConnection);
        QObject::connect(this, &EditTextWidget::dwellEnd, this, &EditTextWidget::dwellEndNotify, Qt::UniqueConnection);
        //    d->client->docHighlightRequest(filePath, lsp::Position{0, 30});
        //    d->client->hoverRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
        //                            lsp::Position{10,0});
        //    d->client->signatureHelpRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
        //                                    lsp::Position{10,0});
        //    d->client->completionRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
        //                                 lsp::Position{10,0});
        //    d->client->definitionRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
        //                                 lsp::Position{10,0});
        //    d->client->symbolRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp");
        //    d->client->referencesRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
        //                                 lsp::Position{10,0});
        //    d->client->highlightRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp",
        //                                lsp::Position{10,0});
        //    d->client->closeRequest("/home/funning/workspace/workspace/recode/gg/unioncode/src/app/main.cpp");

        //    d->client->shutdownRequest();
        //    d->client->exitRequest();
    }
}

void EditTextWidget::publishDiagnostics(const lsp::Diagnostics &diagnostics)
{
    const auto docLen = length();
    indicatorClearRange(0, docLen);
    for (auto val : diagnostics) {
        Sci_Position startPos = getSciPosition(docPointer(), val.range.start);
        Sci_Position endPos = getSciPosition(docPointer(), val.range.end);
        setIndicatorCurrent(val.severity);
        indicatorFillRange(startPos, endPos - startPos);
    }
}

void EditTextWidget::tokenFullResult(const QList<lsp::Data> &tokens)
{
    if (!lexer())
        return;

    if (d->currentLanguageId != lexerLanguage()) {
        ContextDialog::ok(EditTextWidget::tr("There is a fatal error between the current"
                                             " editor component and the backend of the syntax server, "
                                             "which may affect the syntax highlighting. \n"
                                             "Please contact the maintainer for troubleshooting "
                                             "to solve the problem!"));
        return;
    }

    int cacheLine = 0;
    for (auto val : tokens) {
        cacheLine += val.start.line;
        qInfo() << "line:" << val.start.line;
        qInfo() << "charStart:" << val.start.character;
        qInfo() << "charLength:" << val.length;
        qInfo() << "tokenType:" << val.tokenType;
        qInfo() << "tokenModifiers" << val.tokenModifiers;

        if (val.tokenType >= d->tokenTypesCache.size())
            continue;

        QString tokenTypeStr = d->tokenTypesCache[val.tokenType]; // token name
        EditTextWidgetStyle::Indic indic = d->style.tokenKey(tokenTypeStr); //
        setIndicatorCurrent(indic);
        Sci_Position startPos = getSciPosition(docPointer(), {cacheLine, val.start.character});
        indicatorFillRange(startPos, val.length);
    }
}

void EditTextWidget::debugMarginClieced(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin)
{
    Q_UNUSED(modifiers);
    sptr_t line = lineFromPosition(position);
    if (markerGet(line)) {
        SendEvents::marginDebugPointRemove(this->currentFile(), line);
        markerDelete(line, margin);
    } else {
        SendEvents::marginDebugPointAdd(this->currentFile(), line);
        markerAdd(line, 0);
    }
}

void EditTextWidget::tokenDefinitionsSave(const lsp::SemanticTokensProvider &provider)
{
    d->tokenTypesCache = provider.legend.tokenTypes;
    d->tokenModifiersCache = provider.legend.tokenModifiers;
}

void EditTextWidget::completionsSave(const lsp::CompletionProvider &provider)
{
    //    if (!provider.isIncomplete) {
    //        return;
    //    }
    if (d->editInsertCount == 0) {
        return;
    }
    QString labels;
    for (auto item : provider.items) {
        // extension completions for symbol
        if (!item.label.isEmpty()) {
            labels += item.label;
        }
        qInfo() << "\n**************"
                << "\ninsertText:" << item.insertText
                << "\nfilterText:" << item.filterText
                << "\nlabel:" << item.label;
    }
    autoCShow(d->editInsertCount, labels.toUtf8());
}

void EditTextWidget::hoverMessage(const lsp::Hover &hover)
{
    if (d->hoverPos.x() != -1 && d->hoverPos.y() != -1) {
        qInfo() << hover.contents.value;
        callTipSetBack(STYLE_DEFAULT);
        if (!hover.contents.value.isEmpty()) {
            callTipShow(positionFromPoint(d->hoverPos.x(), d->hoverPos.y()),
                        hover.contents.value.toUtf8().toStdString().c_str());
        }
    }
}

void EditTextWidget::sciModified(Scintilla::ModificationFlags type, Scintilla::Position position, Scintilla::Position length,
                                 Scintilla::Position linesAdded, const QByteArray &text, Scintilla::Position line,
                                 Scintilla::FoldLevel foldNow, Scintilla::FoldLevel foldPrev)
{
    Q_UNUSED(position)
    Q_UNUSED(length)
    Q_UNUSED(linesAdded)
    Q_UNUSED(text)
    Q_UNUSED(line)
    Q_UNUSED(foldNow)
    Q_UNUSED(foldPrev)

    if (currentFile().isEmpty()|| !QFile(currentFile()).exists())
        return;

    if (bool(type & Scintilla::ModificationFlags::InsertText)
            || bool(type & Scintilla::ModificationFlags::DeleteText)) {
        d->client->changeRequest(currentFile());
    }

    if (bool(type & Scintilla::ModificationFlags::InsertText)) {

        if (text == " ") {
            d->editInsertPostion = {-1, -1};
            d->editInsertCount = 0;
            return;
        }

        // start
        if (d->editInsertPostion.line == -1 && d->editInsertPostion.character == -1) {
            // autoCCancel();
            d->editInsertPostion = getLspPosition(docPointer(), position);
            d->editInsertCount ++;
            d->client->completionRequest(currentFile(), getLspPosition(docPointer(), position));
            qInfo() << "\n************************"
                    << "\n edit start position:"
                    << "\n  line: " << d->editInsertPostion.line
                    << "\n  character: " << d->editInsertPostion.character
                    << "\n edit insert count:" << d->editInsertCount
                    << "\n************************";
        } else if(d->editInsertPostion.line == -1 || d->editInsertPostion.character == -1) {
            qCritical() << "Error edit text from insertText";
        } else {
            lsp::Position currentPostion = getLspPosition(docPointer(), position);
            if (d->editInsertPostion.line == currentPostion.line
                    && d->editInsertPostion.character + d->editInsertCount == currentPostion.character) {
                d->editInsertCount ++;
                d->client->completionRequest(currentFile(), getLspPosition(docPointer(), position));
                qInfo() << "\n************************"
                        << "\n edit start position:"
                        << "\n  line: " << d->editInsertPostion.line
                        << "\n  character: " << d->editInsertPostion.character
                        << "\n edit insert count:" << d->editInsertCount
                        << "\n************************";
            } else {
                d->editInsertPostion = {-1, -1}; // end edit mode
                d->editInsertCount = 0;
            }
        }
    }

    if (bool(type & Scintilla::ModificationFlags::DeleteText)) {
        if (d->editInsertPostion.line != -1 && d->editInsertPostion.character != -1) {
            lsp::Position currentPostion = getLspPosition(docPointer(), position);
            if (d->editInsertPostion.line == currentPostion.line &&
                    currentPostion.character >= d->editInsertPostion.character) {
                d->editInsertCount --;
                d->client->completionRequest(currentFile(), getLspPosition(docPointer(), position));
                qInfo() << "\n************************"
                        << "\n edit start position:"
                        << "\n  line: " << d->editInsertPostion.line
                        << "\n  character: " << d->editInsertPostion.character
                        << "\n edit insert count:" << d->editInsertCount
                        << "\n************************";
            }
        }
    }
}

void EditTextWidget::dwellStartNotify(int x, int y)
{
    if (d->hoverPos.x() == -1 && d->hoverPos.y() == -1) {
        auto startPosition = positionFromPoint(x, y);
        d->client->docHoverRequest(currentFile(), getLspPosition(docPointer(), startPosition));
        d->hoverPos = {x, y};
    }
}

void EditTextWidget::dwellEndNotify(int x, int y)
{
    if (d->hoverPos.x() == x && d->hoverPos.y() == y) {
        callTipCancel();
        d->hoverPos = {-1, -1};
    }
}

void EditTextWidget::setDefaultStyle()
{
    setMargins(SC_MAX_MARGIN);
    setMarginWidthN(0, 50);
    setMarginSensitiveN(0, SCN_MARGINCLICK);
    setMarginTypeN(0, SC_MARGIN_SYMBOL);
    setMarginMaskN(0, 0x01);    //range mark 1~32
    markerSetFore(0, 0x0000ff); //red
    markerSetBack(0, 0x0000ff); //red
    markerSetAlpha(0, INDIC_GRADIENT);

    setMarginWidthN(1, 20);
    setMarginTypeN(1, SC_MARGIN_NUMBER);
    setMarginMaskN(1, 0x00);   // null
    markerSetFore(1, 0x0000ff); //red
    markerSetBack(1, 0x0000ff); //red
    markerSetAlpha(1, INDIC_GRADIENT);

    styleSetFore(SCE_C_DEFAULT, d->scintillaColor(QColor(0,0,0))); // 空格
    styleSetFore(SCE_C_COMMENT, d->scintillaColor(QColor(255,200,20))); // #整行
    styleSetFore(SCE_C_COMMENTLINE, d->scintillaColor(QColor(0,200,200))); // //注释
    styleSetFore(SCE_C_COMMENTDOC, d->scintillaColor(QColor(255,200,20)));
    styleSetFore(SCE_C_NUMBER, d->scintillaColor(QColor(255,200,20)));
    styleSetFore(SCE_C_WORD, d->scintillaColor(QColor(0,200,0)));
    styleSetFore(SCE_C_STRING, d->scintillaColor(QColor(230,0,255))); // 字符串
    styleSetFore(SCE_C_CHARACTER, d->scintillaColor(QColor(230,0,255)));
    styleSetFore(SCE_C_UUID,d->scintillaColor(QColor(230,0,255)));
    styleSetFore(SCE_C_PREPROCESSOR, d->scintillaColor(QColor(0,0,255))); // #
    styleSetFore(SCE_C_OPERATOR, d->scintillaColor(QColor(0,0,0))); // 符号
    styleSetFore(SCE_C_IDENTIFIER, d->scintillaColor(QColor(0,0,0)));
    styleSetFore(SCE_C_STRINGEOL, d->scintillaColor(QColor(255,200,20)));
    styleSetFore(SCE_C_VERBATIM, d->scintillaColor(QColor(255,200,20)));
    styleSetFore(SCE_C_REGEX, d->scintillaColor(QColor(255,200,20)));
    styleSetFore(SCE_C_COMMENTLINEDOC, d->scintillaColor(QColor(0,200,200))); // ///注释
    styleSetFore(SCE_C_WORD2, d->scintillaColor(QColor(255,200,20))); // 1 一般关键字
    //    styleSetFore(SCE_C_COMMENTDOCKEYWORD, d->scintillaColor(QColor(255,200,20)));
    //    styleSetFore(SCE_C_COMMENTDOCKEYWORDERROR, d->scintillaColor(QColor(0,200,200))); // /// @
    styleSetFore(SCE_C_GLOBALCLASS, d->scintillaColor(QColor(255,200,0))); // 3 关键字
    //    styleSetFore(SCE_C_STRINGRAW, d->scintillaColor(QColor(255,200,0)));
    //    styleSetFore(SCE_C_TRIPLEVERBATIM, d->scintillaColor(QColor(255,200,0)));
    //    styleSetFore(SCE_C_HASHQUOTEDSTRING, d->scintillaColor(QColor(255,200,0)));
    //    styleSetFore(SCE_C_PREPROCESSORCOMMENT, d->scintillaColor(QColor(255,200,0)));
    //    styleSetFore(SCE_C_PREPROCESSORCOMMENTDOC, d->scintillaColor(QColor(255,200,0)));
    //    styleSetFore(SCE_C_USERLITERAL, d->scintillaColor(QColor(255,200,0)));
    //    styleSetFore(SCE_C_TASKMARKER, d->scintillaColor(QColor(255,200,0)));
    //    styleSetFore(SCE_C_ESCAPESEQUENCE, d->scintillaColor(QColor(255,200,0)));
}

bool EditTextWidget::setLspIndicStyle(const QString &languageID)
{
    typedef EditTextWidgetStyle::Indic Indic;
    indicSetStyle(Indic::DiagnosticUnkown, INDIC_HIDDEN);
    indicSetStyle(Indic::DiagnosticError, INDIC_SQUIGGLE);
    indicSetStyle(Indic::DiagnosticWarning, INDIC_SQUIGGLE);
    indicSetStyle(Indic::DiagnosticInfo, INDIC_PLAIN);
    indicSetStyle(Indic::DiagnosticHint, INDIC_PLAIN);

    indicSetFore(Indic::DiagnosticUnkown, 0x000000);
    indicSetFore(Indic::DiagnosticError, 0x0000ff);
    indicSetFore(Indic::DiagnosticWarning, 0x0000ff);
    indicSetFore(Indic::DiagnosticInfo, 0x00ffff);
    indicSetFore(Indic::DiagnosticHint, 0x00ffff);

    indicSetStyle(Indic::TokenTypeNamespace, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeType, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeClass, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeEnum, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeInterface, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeStruct, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeTypeParameter, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeVariable, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeProperty, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeEnumMember, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeEvent, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeFunction, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeMethod, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeKeyword, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeModifier, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeComment, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeString, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeNumber, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeRegexp, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenTypeOperator, INDIC_TEXTFORE);

    indicSetStyle(Indic::TokenModifierDeclaration, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenModifierDefinition, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenModifierReadonly, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenModifierStatic, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenModifierDeprecated, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenModifierAbstract, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenModifierAsync, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenModifierModification, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenModifierDocumentation, INDIC_TEXTFORE);
    indicSetStyle(Indic::TokenModifierDefaultLibrary, INDIC_TEXTFORE);

    //set language keys style
    QStringList themes = d->style.styleThemes(languageID);
    QList<Indic> lspKeys = d->style.lspKeys();
    foreach (auto lspKey, lspKeys) {
        //need user cache setting file set theme
        if (themes.isEmpty())
            break;

        this->cursor();
        QCursor cursor = QWidget::cursor();
        auto foreground = d->style.foreground(languageID, themes.first(), d->style.styleValue(lspKey));
        auto background = d->style.background(languageID, themes.first(), d->style.styleValue(lspKey));
        // indicSetFore(lspKey, d->scintillaColor(QColor(Qt::red)));
        indicSetFore(lspKey, d->scintillaColor(QColor(foreground)));
        // styleSetFont(lspKey,) 预留
    }
    return true;
}

