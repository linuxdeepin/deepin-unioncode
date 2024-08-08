// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "languageclienthandler.h"
#include "private/languageclienthandler_p.h"
#include "gui/texteditor.h"
#include "gui/settings/editorsettings.h"
#include "gui/settings/settingsdefine.h"
#include "lspclientmanager.h"
#include "transceiver/codeeditorreceiver.h"
#include "codelens/codelens.h"
#include "symbol/symbolmanager.h"

#include "services/project/projectservice.h"

#include "Qsci/qscilexer.h"

#include <DApplicationHelper>
#include <QApplication>

#include <bitset>

DGUI_USE_NAMESPACE

LanguageClientHandlerPrivate::LanguageClientHandlerPrivate(TextEditor *edit, LanguageClientHandler *qq)
    : q(qq),
      editor(edit)
{
}

LanguageClientHandlerPrivate::~LanguageClientHandlerPrivate()
{
    languageWorker->stop();
    thread.quit();
    thread.wait();
    languageWorker->deleteLater();
}

void LanguageClientHandlerPrivate::init()
{
    diagnosticFormat = "%1\n%2:%3";
    textChangedTimer.setSingleShot(true);
    textChangedTimer.setInterval(200);

    positionChangedTimer.setSingleShot(true);
    positionChangedTimer.setInterval(250);

    languageWorker = new LanguageWorker(q);
    languageWorker->setTextEditor(editor);
    languageWorker->moveToThread(&thread);
    thread.start();

    initConnection();
    initLspConnection();
    initIndicStyle();
}

void LanguageClientHandlerPrivate::initConnection()
{
    connect(editor, &TextEditor::textChanged, this, [this] { textChangedTimer.start(); });
    connect(editor, &TextEditor::cursorPositionChanged, this, [this] { positionChangedTimer.start(); });
    connect(editor, &TextEditor::documentHovered, this, &LanguageClientHandlerPrivate::handleHoveredStart);
    connect(editor, &TextEditor::documentHoverEnd, this, &LanguageClientHandlerPrivate::handleHoverEnd);
    connect(editor, &TextEditor::requestFollowType, this, &LanguageClientHandlerPrivate::handleFollowTypeStart);
    connect(editor, &TextEditor::followTypeEnd, this, &LanguageClientHandlerPrivate::handleFollowTypeEnd);
    connect(editor, &TextEditor::indicatorClicked, this, &LanguageClientHandlerPrivate::handleIndicClicked);
    connect(editor, &TextEditor::contextMenuRequested, this, &LanguageClientHandlerPrivate::handleShowContextMenu);
    connect(editor, &TextEditor::fileClosed, this, &LanguageClientHandlerPrivate::handleFileClosed);
    connect(&renamePopup, &RenamePopup::editingFinished, this, &LanguageClientHandlerPrivate::handleRename);
    connect(&textChangedTimer, &QTimer::timeout, this, &LanguageClientHandlerPrivate::delayTextChanged);
    connect(&positionChangedTimer, &QTimer::timeout, this, &LanguageClientHandlerPrivate::delayPositionChanged);
    connect(languageWorker, &LanguageWorker::highlightToken, this, &LanguageClientHandlerPrivate::handleHighlightToken);
}

void LanguageClientHandlerPrivate::initLspConnection()
{
    auto client = getClient();
    if (!editor || !client)
        return;

    auto referencesResult = qOverload<const lsp::References &>(&newlsp::Client::requestResult);
    connect(client, referencesResult, CodeLens::instance(), &CodeLens::displayReference, Qt::UniqueConnection);
    connect(client, &newlsp::Client::switchHeaderSourceResult, this, &LanguageClientHandlerPrivate::handleSwitchHeaderSource, Qt::UniqueConnection);

    //bind signals to file diagnostics
    connect(client, &newlsp::Client::publishDiagnostics, this, &LanguageClientHandlerPrivate::handleDiagnostics);

    auto tokenResult = qOverload<const QList<lsp::Data> &, const QString &>(&newlsp::Client::requestResult);
    connect(client, tokenResult, this, &LanguageClientHandlerPrivate::handleTokenFull);

    auto completeResult = qOverload<const lsp::CompletionProvider &>(&newlsp::Client::requestResult);
    connect(client, completeResult, q, &LanguageClientHandler::completeFinished);

    connect(client, &newlsp::Client::hoverRes, this, &LanguageClientHandlerPrivate::handleShowHoverInfo);
    connect(client, &newlsp::Client::renameRes, EditorCallProxy::instance(), &EditorCallProxy::reqDoRename, Qt::UniqueConnection);
    connect(client, &newlsp::Client::rangeFormattingRes, this, &LanguageClientHandlerPrivate::handleRangeFormattingReplace);
    connect(client, &newlsp::Client::documentHighlightResult, this, &LanguageClientHandlerPrivate::handleDocumentHighlight);

    /* to use QOverload cast virtual slot can't working */
    connect(client, qOverload<const newlsp::Location &, const QString &>(&newlsp::Client::definitionRes),
            this, qOverload<const newlsp::Location &, const QString &>(&LanguageClientHandlerPrivate::handleCodeDefinition));
    connect(client, qOverload<const std::vector<newlsp::Location> &, const QString &>(&newlsp::Client::definitionRes),
            this, qOverload<const std::vector<newlsp::Location> &, const QString &>(&LanguageClientHandlerPrivate::handleCodeDefinition));
    connect(client, qOverload<const std::vector<newlsp::LocationLink> &, const QString &>(&newlsp::Client::definitionRes),
            this, qOverload<const std::vector<newlsp::LocationLink> &, const QString &>(&LanguageClientHandlerPrivate::handleCodeDefinition));

    // symbol
    auto docSymbolResult = qOverload<const QList<newlsp::DocumentSymbol> &, const QString &>(&newlsp::Client::symbolResult);
    connect(client, docSymbolResult, this, &LanguageClientHandlerPrivate::handleDocumentSymbolResult);
    auto symbolInfoResult = qOverload<const QList<newlsp::SymbolInformation> &, const QString &>(&newlsp::Client::symbolResult);
    connect(client, symbolInfoResult, this, &LanguageClientHandlerPrivate::handleSymbolInfomationResult);
}

void LanguageClientHandlerPrivate::initIndicStyle()
{
    editor->indicatorDefine(TextEditor::PlainIndicator, TextEditor::INDIC_PLAIN);
    editor->indicatorDefine(TextEditor::SquiggleIndicator, TextEditor::INDIC_SQUIGGLE);
    editor->indicatorDefine(TextEditor::TTIndicator, TextEditor::INDIC_TT);
    editor->indicatorDefine(TextEditor::DiagonalIndicator, TextEditor::INDIC_DIAGONAL);
    editor->indicatorDefine(TextEditor::StrikeIndicator, TextEditor::INDIC_STRIKE);
    editor->indicatorDefine(TextEditor::HiddenIndicator, TextEditor::INDIC_HIDDEN);
    editor->indicatorDefine(TextEditor::BoxIndicator, TextEditor::INDIC_BOX);
    editor->indicatorDefine(TextEditor::RoundBoxIndicator, TextEditor::INDIC_ROUNDBOX);
    editor->indicatorDefine(TextEditor::StraightBoxIndicator, TextEditor::INDIC_STRAIGHTBOX);
    editor->indicatorDefine(TextEditor::FullBoxIndicator, TextEditor::INDIC_FULLBOX);
    editor->indicatorDefine(TextEditor::DashesIndicator, TextEditor::INDIC_DASH);
    editor->indicatorDefine(TextEditor::DotsIndicator, TextEditor::INDIC_DOTS);
    editor->indicatorDefine(TextEditor::SquiggleLowIndicator, TextEditor::INDIC_SQUIGGLELOW);
    editor->indicatorDefine(TextEditor::DotBoxIndicator, TextEditor::INDIC_DOTBOX);
    editor->indicatorDefine(TextEditor::GradientIndicator, TextEditor::INDIC_GRADIENT);
    editor->indicatorDefine(TextEditor::GradientIndicator, TextEditor::INDIC_GRADIENTCENTRE);
    editor->indicatorDefine(TextEditor::SquigglePixmapIndicator, TextEditor::INDIC_SQUIGGLEPIXMAP);
    editor->indicatorDefine(TextEditor::ThickCompositionIndicator, TextEditor::INDIC_COMPOSITIONTHICK);
    editor->indicatorDefine(TextEditor::ThinCompositionIndicator, TextEditor::INDIC_COMPOSITIONTHIN);
    editor->indicatorDefine(TextEditor::TextColorIndicator, TextEditor::INDIC_TEXTFORE);
    editor->indicatorDefine(TextEditor::TriangleIndicator, TextEditor::INDIC_POINT);
    editor->indicatorDefine(TextEditor::TriangleCharacterIndicator, TextEditor::INDIC_POINTCHARACTER);
}

QString LanguageClientHandlerPrivate::formatDiagnosticMessage(const QString &message, int type)
{
    auto result = message;
    switch (type) {
    case AnnotationType::ErrorAnnotation:
        result = diagnosticFormat.arg("Parse Issue", "Error", result);
        break;
    default:
        break;
    }

    return result;
}

bool LanguageClientHandlerPrivate::shouldStartCompletion(const QString &insertedText)
{
    if (insertedText.isEmpty())
        return false;

    QChar lastChar = insertedText.at(insertedText.count() - 1);
    if (lastChar.isLetter() || lastChar.isNumber() || lastChar == QLatin1Char('_'))
        return true;

    if (editor && editor->lexer()) {
        auto spList = editor->lexer()->autoCompletionWordSeparators();
        auto iter = std::find_if(spList.begin(), spList.end(),
                                 [&insertedText](const QString &sp) {
                                     return insertedText.endsWith(sp);
                                 });
        return iter != spList.end();
    }

    return false;
}

int LanguageClientHandlerPrivate::wordPostion()
{
    int pos = editor->cursorPosition();
    if (editor->hasSelectedText()) {
        int startPos = editor->wordStartPositoin(pos);
        int endPos = editor->wordEndPosition(pos);
        return (startPos + endPos) / 2;
    }

    return pos;
}

newlsp::Client *LanguageClientHandlerPrivate::getClient()
{
    if (projectKey.isValid())
        return LSPClientManager::instance()->get(projectKey);

    auto prjSrv = dpfGetService(dpfservice::ProjectService);
    const auto &filePath = editor->getFile();
    const auto &allProject = prjSrv->getAllProjectInfo();
    for (const auto &prj : allProject) {
        const auto &files = prj.sourceFiles();
        if (!files.contains(filePath))
            continue;

        projectKey.workspace = prj.workspaceFolder().toStdString();
        projectKey.outputDirectory = prj.buildFolder().toStdString();
        break;
    }

    if (projectKey.workspace.empty()) {
        auto prj = prjSrv->getActiveProjectInfo();
        projectKey.workspace = prj.workspaceFolder().toStdString();
        projectKey.outputDirectory = prj.buildFolder().toStdString();
    }

    auto id = support_file::Language::id(filePath);
    projectKey.language = support_file::Language::idAlias(id).toStdString();

    return LSPClientManager::instance()->get(projectKey);
}

void LanguageClientHandlerPrivate::handleDiagnostics(const newlsp::PublishDiagnosticsParams &data)
{
    if (!editor)
        return;

    if (QUrl(QString::fromStdString(data.uri)).toLocalFile() != editor->getFile())
        return;

    // clear all flags of diagnostics
    editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_SQUIGGLE);
    editor->SendScintilla(TextEditor::SCI_INDICATORCLEARRANGE, 0, editor->length());
    this->cleanDiagnostics();
    for (auto val : data.diagnostics) {
        if (newlsp::Enum::DiagnosticSeverity::get()->Error == val.severity.value()) {   // error
            newlsp::Position start { val.range.start.line, val.range.start.character };
            newlsp::Position end { val.range.end.line, val.range.end.character };
            int startPos = editor->positionFromLineIndex(start.line, start.character);
            int endPos = editor->positionFromLineIndex(end.line, end.character);

            editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_SQUIGGLE);
            editor->SendScintilla(TextEditor::SCI_INDICSETFORE, TextEditor::INDIC_SQUIGGLE, QColor(Qt::red));
            editor->SendScintilla(TextEditor::SCI_INDICATORFILLRANGE, static_cast<ulong>(startPos), endPos - startPos);

            std::string message = val.message.value();
            diagnosticCache.append({ startPos, endPos, message.c_str(), AnnotationType::ErrorAnnotation });
        }
    }
}

void LanguageClientHandlerPrivate::cleanDiagnostics()
{
    diagnosticCache.clear();
}

void LanguageClientHandlerPrivate::handleTokenFull(const QList<lsp::Data> &tokens, const QString &filePath)
{
    if (!editor || editor->getFile() != filePath || !editor->lexer())
        return;

    metaObject()->invokeMethod(languageWorker,
                               "handleDocumentSemanticTokens",
                               Qt::QueuedConnection,
                               Q_ARG(QList<lsp::Data>, tokens));
}

void LanguageClientHandlerPrivate::handleShowHoverInfo(const newlsp::Hover &hover)
{
    if (!editor || hoverCache.getPosition() == -1)
        return;

    std::string showText;
    if (newlsp::any_contrast<std::vector<newlsp::MarkedString>>(hover.contents)) {
        auto markupStrings = std::any_cast<std::vector<newlsp::MarkedString>>(hover.contents);
        for (auto one : markupStrings) {
            if (!showText.empty()) showText += "\n";

            if (!one.value.empty())   // markedString value append
                showText += one.value;
            else if (!std::string(one).empty())   // markedString self is String append
                showText += one;
        };
    } else if (newlsp::any_contrast<newlsp::MarkupContent>(hover.contents)) {
        auto markupContent = std::any_cast<newlsp::MarkupContent>(hover.contents);
        showText = markupContent.value;
    } else if (newlsp::any_contrast<newlsp::MarkedString>(hover.contents)) {
        auto markedString = std::any_cast<newlsp::MarkedString>(hover.contents);
        if (!std::string(markedString).empty()) {
            showText = std::string(markedString);
        } else {
            showText = markedString.value;
        }
    }

    if (!showText.empty())
        editor->showTips(hoverCache.getPosition(), showText.c_str());
}

void LanguageClientHandlerPrivate::handleCodeDefinition(const newlsp::Location &data, const QString &filePath)
{
    if (!editor || editor->getFile() != filePath)
        return;

    definitionCache.set(data);
    if (definitionCache.switchMode() == DefinitionCache::ClickMode) {
        auto textRange = definitionCache.getTextRange();
        setDefinitionSelectedStyle(textRange.getStart(), textRange.getEnd());
    } else {
        gotoDefinition();
        definitionCache.clean();
    }
}

void LanguageClientHandlerPrivate::handleCodeDefinition(const std::vector<newlsp::Location> &data, const QString &filePath)
{
    if (!editor || data.empty() || editor->getFile() != filePath)
        return;

    definitionCache.set(data);
    if (definitionCache.switchMode() == DefinitionCache::ClickMode) {
        auto textRange = definitionCache.getTextRange();
        if (!textRange.isEmpty())
            setDefinitionSelectedStyle(textRange.getStart(), textRange.getEnd());
    } else {
        gotoDefinition();
        definitionCache.clean();
    }
}

void LanguageClientHandlerPrivate::handleCodeDefinition(const std::vector<newlsp::LocationLink> &data, const QString &filePath)
{
    if (!editor || data.empty() || editor->getFile() != filePath)
        return;

    definitionCache.set(data);
    if (definitionCache.switchMode() == DefinitionCache::ClickMode) {
        auto textRange = definitionCache.getTextRange();
        setDefinitionSelectedStyle(textRange.getStart(), textRange.getEnd());
    } else {
        gotoDefinition();
        definitionCache.clean();
    }
}

void LanguageClientHandlerPrivate::cleanDefinition(int pos)
{
    auto data = editor->SendScintilla(TextEditor::SCI_INDICATORALLONFOR, pos);
    std::bitset<32> flags(static_cast<ulong>(data));
    if (flags[TextEditor::INDIC_COMPOSITIONTHICK]) {
        editor->SendScintilla(TextEditor::SCI_SETCURSOR, definitionCache.getCursor());
        editor->SendScintilla(TextEditor::SCI_INDICATORCLEARRANGE, 0, editor->length());
    }
}

void LanguageClientHandlerPrivate::handleRangeFormattingReplace(const std::vector<newlsp::TextEdit> &edits, const QString &filePath)
{
    if (edits.empty() || !editor || editor->getFile() != filePath)
        return;

    for (auto itera = edits.rbegin(); itera != edits.rend(); itera++) {
        editor->replaceRange(itera->range.start.line, itera->range.start.character,
                             itera->range.end.line, itera->range.end.character,
                             QString::fromStdString(itera->newText));
    }
}

void LanguageClientHandlerPrivate::setDefinitionSelectedStyle(int start, int end)
{
    editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_COMPOSITIONTHICK);
    editor->SendScintilla(TextEditor::SCI_INDICATORFILLRANGE, static_cast<ulong>(start), end - start);

    auto cursor = editor->SendScintilla(TextEditor::SCI_GETCURSOR);
    if (cursor != 8) {
        definitionCache.setCursor(static_cast<int>(cursor));
        editor->SendScintilla(TextEditor::SCI_SETCURSOR, 8);   // hand from Scintilla platfrom.h
    }
}

void LanguageClientHandlerPrivate::delayTextChanged()
{
    if (!editor)
        return;

    cleanDiagnostics();
    if (auto client = getClient()) {
        const auto &content = editor->text();
        client->changeRequest(editor->getFile(), content.toUtf8());
        client->docSemanticTokensFull(editor->getFile());
        client->symbolRequest(editor->getFile());
    }
}

void LanguageClientHandlerPrivate::delayPositionChanged()
{
    if (!editor || !getClient())
        return;

    lsp::Position pos;
    editor->lineIndexFromPosition(wordPostion(), &pos.line, &pos.character);
    getClient()->docHighlightRequest(editor->getFile(), pos);
}

void LanguageClientHandlerPrivate::handleHoveredStart(int position)
{
    if (!editor || !getClient())
        return;

    if (!diagnosticCache.isEmpty()) {
        auto iter = std::find_if(diagnosticCache.begin(), diagnosticCache.end(),
                                 [position](const DiagnosticCache &cache) {
                                     return cache.contains(position);
                                 });
        if (iter != diagnosticCache.end()) {
            const auto &msg = formatDiagnosticMessage(iter->message, iter->type);
            editor->showTips(position, msg);
            return;
        }
    }

    hoverCache.setPosition(position);
    auto textRange = hoverCache.getTextRange();
    if (!textRange.isEmpty() && textRange.contaions(position))
        return;

    auto startPos = editor->SendScintilla(TextEditor::SCI_WORDSTARTPOSITION, static_cast<ulong>(position), true);
    auto endPos = editor->SendScintilla(TextEditor::SCI_WORDENDPOSITION, static_cast<ulong>(position), true);
    // hover in the empty area
    if (startPos == endPos)
        return;

    hoverCache.setTextRange(static_cast<int>(startPos), static_cast<int>(endPos));
    lsp::Position pos;
    editor->lineIndexFromPosition(position, &pos.line, &pos.character);
    getClient()->docHoverRequest(editor->getFile(), pos);
}

void LanguageClientHandlerPrivate::handleHoverEnd(int position)
{
    if (!editor)
        return;

    auto textRange = hoverCache.getTextRange();
    if (!textRange.isEmpty() && !hoverCache.getTextRange().contaions(position)) {
        editor->cancelTips();
        hoverCache.clean();
    }
}

void LanguageClientHandlerPrivate::handleFollowTypeStart(int position)
{
    if (!editor || editor->wordAtPosition(position).isEmpty()) {
        handleFollowTypeEnd();
        return;
    }

    auto startPos = editor->SendScintilla(TextEditor::SCI_WORDSTARTPOSITION, static_cast<ulong>(position), true);
    auto endPos = editor->SendScintilla(TextEditor::SCI_WORDENDPOSITION, static_cast<ulong>(position), true);
    RangeCache textRange { static_cast<int>(startPos), static_cast<int>(endPos) };

    if (definitionCache.getTextRange() == textRange)
        return;

    handleFollowTypeEnd();
    definitionCache.setPosition((startPos + endPos) / 2);
    definitionCache.setTextRange(textRange);
    definitionCache.cleanFromLsp();
    definitionCache.setSwitchMode(DefinitionCache::ClickMode);

    lsp::Position pos;
    editor->lineIndexFromPosition(position, &pos.line, &pos.character);
    if (getClient())
        getClient()->definitionRequest(editor->getFile(), pos);
}

void LanguageClientHandlerPrivate::handleFollowTypeEnd()
{
    if (!editor || definitionCache.getTextRange().isEmpty())
        return;

    cleanDefinition(definitionCache.getPosition());
    definitionCache.clean();
}

void LanguageClientHandlerPrivate::handleIndicClicked(int line, int index)
{
    if (!editor)
        return;

    auto pos = editor->positionFromLineIndex(line, index);
    auto data = editor->SendScintilla(TextEditor::SCI_INDICATORALLONFOR, pos);
    std::bitset<32> flags(static_cast<ulong>(data));
    if (flags[TextEditor::INDIC_COMPOSITIONTHICK]) {
        gotoDefinition();
        cleanDefinition(pos);
    }
}

void LanguageClientHandlerPrivate::handleShowContextMenu(QMenu *menu)
{
    if (!editor)
        return;

    auto actionList = menu->actions();
    for (auto act : actionList) {
        if (act->text() == tr("Refactor")) {
            QMenu *subMenu = new QMenu(menu);
            subMenu->addAction(tr("Rename Symbol Under Cursor"), q, &LanguageClientHandler::renameActionTriggered);
            act->setMenu(subMenu);
            break;
        }
    }

    auto act = menu->addAction(tr("Switch Header/Source"), q, std::bind(&LanguageClientHandler::switchHeaderSource, q, editor->getFile()));
    menu->insertAction(actionList.first(), act);

    act = menu->addAction(tr("Follow Symbol Under Cursor"), q, &LanguageClientHandler::followSymbolUnderCursor);
    menu->insertAction(actionList.first(), act);

    act = menu->addAction(tr("Find Usages"), q, &LanguageClientHandler::findUsagesActionTriggered);
    menu->insertAction(actionList.first(), act);

    act = menu->addAction(tr("Format Selection"), q, &LanguageClientHandler::formatSelections);
    menu->insertAction(actionList.first(), act);
    menu->insertSeparator(actionList.first());
}

void LanguageClientHandlerPrivate::handleFileClosed(const QString &file)
{
    if (getClient())
        getClient()->closeRequest(file);
}

void LanguageClientHandlerPrivate::handleRename(const QString &text)
{
    if (!editor || !getClient() || !renameCache.isValid())
        return;

    lsp::Position pos { renameCache.line, renameCache.column };
    getClient()->renameRequest(editor->getFile(), pos, text);
    renameCache.clear();
}

void LanguageClientHandlerPrivate::gotoDefinition()
{
    if (definitionCache.getLocations().size() > 0) {
        Q_EMIT editor->cursorRecordChanged(editor->cursorLastPosition());
        auto one = definitionCache.getLocations().front();
        EditorCallProxy::instance()->reqGotoPosition(QUrl(QString::fromStdString(one.uri)).toLocalFile(),
                                                     one.range.start.line, one.range.start.character);
    } else if (definitionCache.getLocationLinks().size() > 0) {
        Q_EMIT editor->cursorRecordChanged(editor->cursorLastPosition());
        auto one = definitionCache.getLocationLinks().front();
        EditorCallProxy::instance()->reqGotoPosition(QUrl(QString::fromStdString(one.targetUri)).toLocalFile(),
                                                     one.targetRange.end.line, one.targetRange.end.character);
    } else {
        Q_EMIT editor->cursorRecordChanged(editor->cursorLastPosition());
        auto one = definitionCache.getLocation();
        EditorCallProxy::instance()->reqGotoPosition(QUrl(QString::fromStdString(one.uri)).toLocalFile(),
                                                     one.range.start.line, one.range.start.character);
    }
}

void LanguageClientHandlerPrivate::handleSwitchHeaderSource(const QString &file)
{
    if (file.isEmpty())
        return;

    emit EditorCallProxy::instance()->reqOpenFile("", file);
}

void LanguageClientHandlerPrivate::handleDocumentSymbolResult(const QList<newlsp::DocumentSymbol> &docSymbols,
                                                              const QString &filePath)
{
    if (!editor || editor->getFile() != filePath)
        return;

    SymbolManager::instance()->setDocumentSymbols(filePath, docSymbols);
}

void LanguageClientHandlerPrivate::handleSymbolInfomationResult(const QList<newlsp::SymbolInformation> &symbolInfos,
                                                                const QString &filePath)
{
    if (!editor || editor->getFile() != filePath)
        return;

    SymbolManager::instance()->setSymbolInformations(filePath, symbolInfos);
}

void LanguageClientHandlerPrivate::handleDocumentHighlight(const QList<newlsp::DocumentHighlight> &docHighlightList,
                                                           const QString &filePath)
{
    if (!editor || editor->getFile() != filePath)
        return;

    editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_FULLBOX);
    editor->SendScintilla(TextEditor::SCI_INDICATORCLEARRANGE, 0, editor->length());
    QColor forgColor(0, 0, 0, 30);
    QColor lineColor(0, 0, 0, 100);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        forgColor.setRgb(200, 200, 200, 40);
        lineColor.setRgb(200, 200, 200, 100);
    }
    editor->setIndicatorForegroundColor(forgColor, TextEditor::INDIC_FULLBOX);
    editor->setIndicatorOutlineColor(lineColor, TextEditor::INDIC_FULLBOX);
    for (const auto &dh : docHighlightList) {
        int startPos = editor->positionFromLineIndex(dh.range.start.line, dh.range.start.character);
        int endPos = editor->positionFromLineIndex(dh.range.end.line, dh.range.end.character);
        editor->SendScintilla(TextEditor::SCI_INDICATORFILLRANGE, static_cast<ulong>(startPos), endPos - startPos);
    }
}

void LanguageClientHandlerPrivate::handleHighlightToken(const QList<LanguageWorker::DocumentToken> &tokenList)
{
    // clear all text color
    editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_TEXTFORE);
    editor->SendScintilla(TextEditor::SCI_INDICATORCLEARRANGE, 0, editor->length());
    for (const auto &token : tokenList) {
        editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_TEXTFORE);
        editor->SendScintilla(TextEditor::SCI_INDICSETFLAGS, TextEditor::INDIC_TEXTFORE, 1);
        editor->SendScintilla(TextEditor::SCI_SETINDICATORVALUE, token.color);
        editor->SendScintilla(TextEditor::SCI_INDICATORFILLRANGE,
                              static_cast<ulong>(token.startPostion),
                              token.field.length());
    }
}

LanguageClientHandler::LanguageClientHandler(TextEditor *parent)
    : QObject(parent),
      d(new LanguageClientHandlerPrivate(parent, this))
{
    d->init();
}

LanguageClientHandler::~LanguageClientHandler()
{
}

void LanguageClientHandler::requestCompletion(int line, int column)
{
    if (!d->getClient())
        return;

    lsp::CompletionContext context;
    if (d->editor->lexer()) {
        int pos = d->editor->positionFromLineIndex(line, column);
        if (pos > 0) {
            char ch = d->editor->SendScintilla(TextEditor::SCI_GETCHARAT, --pos);
            auto spList = d->editor->lexer()->autoCompletionWordSeparators();
            auto iter = std::find_if(spList.begin(), spList.end(),
                                     [&ch](const QString &sp) {
                                         return sp.endsWith(ch);
                                     });
            if (iter != spList.end()) {
                context.kind = lsp::CompletionTriggerKind::TriggerCharacter;
                context.triggerCharacter = ch;
            }
        }
    }

    lsp::Position pos { line, column };
    d->getClient()->completionRequest(d->editor->getFile(), pos, context);
}

void LanguageClientHandler::updateTokens()
{
    if (auto client = d->getClient()) {
        client->openRequest(d->editor->getFile());
        client->docSemanticTokensFull(d->editor->getFile());
        client->symbolRequest(d->editor->getFile());
    }
}

lsp::SemanticTokenType::type_value LanguageClientHandler::tokenToDefine(int token)
{
    auto client = d->getClient();
    if (!client)
        return {};
    auto initSecTokensProvider = client->initSecTokensProvider();
    if (0 <= token && token < initSecTokensProvider.legend.tokenTypes.size())
        return initSecTokensProvider.legend.tokenTypes[token];
    return {};
}

QColor LanguageClientHandler::symbolIndicColor(lsp::SemanticTokenType::type_value token,
                                               QList<lsp::SemanticTokenType::type_index> modifier)
{
    Q_UNUSED(modifier);
    QMap<int, QColor> result;

    const auto &filePath = d->editor->getFile();
    auto langId = support_file::Language::id(filePath);

    return LSPClientManager::instance()->highlightColor(langId, token);
}

void LanguageClientHandler::refreshTokens()
{
    if (!d->editor || !d->getClient())
        return;

    d->getClient()->docSemanticTokensFull(d->editor->getFile());
}

void LanguageClientHandler::switchHeaderSource(const QString &file)
{
    if (!d->getClient())
        return;

    d->getClient()->switchHeaderSource(file);
}

void LanguageClientHandler::followSymbolUnderCursor()
{
    if (!d->editor || !d->editor->hasFocus() || !d->getClient())
        return;

    d->definitionCache.setSwitchMode(DefinitionCache::ActionMode);

    lsp::Position pos;
    d->editor->lineIndexFromPosition(d->wordPostion(), &pos.line, &pos.character);
    d->getClient()->definitionRequest(d->editor->getFile(), pos);
}

void LanguageClientHandler::findUsagesActionTriggered()
{
    if (!d->editor || !d->getClient())
        return;

    lsp::Position pos;
    d->editor->lineIndexFromPosition(d->wordPostion(), &pos.line, &pos.character);
    d->getClient()->referencesRequest(d->editor->getFile(), pos);
}

void LanguageClientHandler::renameActionTriggered()
{
    if (!d->editor)
        return;

    int pos = d->editor->cursorPosition();
    const auto &symbol = d->editor->wordAtPosition(pos);
    if (symbol.isEmpty())
        return;

    d->editor->lineIndexFromPosition(pos, &d->renameCache.line, &d->renameCache.column);
    auto point = d->editor->pointFromPosition(pos);
    point = d->editor->mapToGlobal(point);

    d->renamePopup.setOldName(symbol);
    d->renamePopup.exec(point);
}

void LanguageClientHandler::formatSelections()
{
    if (!d->getClient() || !d->editor || !d->editor->hasSelectedText())
        return;

    int lineFrom, indexFrom, lineTo, indexTo;
    d->editor->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

    newlsp::Position selStart { lineFrom, indexFrom };
    newlsp::Position selEnd { lineTo, indexTo };
    newlsp::DocumentRangeFormattingParams params;
    params.textDocument.uri = QUrl::fromLocalFile(d->editor->getFile()).toString().toStdString();
    params.range = { selStart, selEnd };
    params.options.tabSize = EditorSettings::instance()->value(Node::Behavior, Group::TabGroup, Key::TabSize, 4).toInt();
    params.options.insertSpaces = true;

    d->getClient()->rangeFormatting(d->editor->getFile(), params);
}
