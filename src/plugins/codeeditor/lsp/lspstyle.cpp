// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lspstyle.h"
#include "private/lspstyle_p.h"
#include "gui/texteditor.h"
#include "gui/settings/editorsettings.h"
#include "gui/settings/settingsdefine.h"
#include "lspclientmanager.h"
#include "transceiver/codeeditorreceiver.h"
#include "codelens/codelens.h"

#include "services/project/projectservice.h"

#include "Qsci/qscilexer.h"

#include <DApplicationHelper>
#include <QApplication>

#include <bitset>

DGUI_USE_NAMESPACE

LSPStylePrivate::LSPStylePrivate(TextEditor *edit, LSPStyle *qq)
    : q(qq),
      editor(edit)
{
    diagnosticFormat = "%1\n%2:%3";
    textChangedTimer.setSingleShot(true);
    textChangedTimer.setInterval(200);

    positionChangedTimer.setSingleShot(true);
    positionChangedTimer.setInterval(250);
}

void LSPStylePrivate::initConnection()
{
    connect(editor, &TextEditor::textChanged, this, [this] { textChangedTimer.start(); });
    connect(editor, &TextEditor::cursorPositionChanged, this, [this] { positionChangedTimer.start(); });
    connect(editor, &TextEditor::documentHovered, this, &LSPStylePrivate::handleHoveredStart);
    connect(editor, &TextEditor::documentHoverEnd, this, &LSPStylePrivate::handleHoverEnd);
    connect(editor, &TextEditor::requestFollowType, this, &LSPStylePrivate::handleFollowTypeStart);
    connect(editor, &TextEditor::followTypeEnd, this, &LSPStylePrivate::handleFollowTypeEnd);
    connect(editor, &TextEditor::indicatorClicked, this, &LSPStylePrivate::handleIndicClicked);
    connect(editor, &TextEditor::contextMenuRequested, this, &LSPStylePrivate::handleShowContextMenu);
    connect(editor, &TextEditor::fileClosed, this, &LSPStylePrivate::handleFileClosed);
    connect(&renamePopup, &RenamePopup::editingFinished, this, &LSPStylePrivate::handleRename);
    connect(&textChangedTimer, &QTimer::timeout, this, &LSPStylePrivate::delayTextChanged);
    connect(&positionChangedTimer, &QTimer::timeout, this, &LSPStylePrivate::delayPositionChanged);
}

void LSPStylePrivate::initLspConnection()
{
    auto client = getClient();
    if (!editor || !client)
        return;

    auto referencesResult = qOverload<const lsp::References &>(&newlsp::Client::requestResult);
    connect(client, referencesResult, CodeLens::instance(), &CodeLens::displayReference, Qt::UniqueConnection);
    connect(client, &newlsp::Client::switchHeaderSourceResult, this, &LSPStylePrivate::handleSwitchHeaderSource, Qt::UniqueConnection);

    //bind signals to file diagnostics
    connect(client, &newlsp::Client::publishDiagnostics, this, &LSPStylePrivate::handleDiagnostics);

    auto tokenResult = qOverload<const QList<lsp::Data> &, const QString &>(&newlsp::Client::requestResult);
    connect(client, tokenResult, this, &LSPStylePrivate::handleTokenFull);

    auto completeResult = qOverload<const lsp::CompletionProvider &>(&newlsp::Client::requestResult);
    connect(client, completeResult, q, &LSPStyle::completeFinished);

    connect(client, &newlsp::Client::hoverRes, this, &LSPStylePrivate::handleShowHoverInfo);
    connect(client, &newlsp::Client::renameRes, EditorCallProxy::instance(), &EditorCallProxy::reqDoRename, Qt::UniqueConnection);
    connect(client, &newlsp::Client::rangeFormattingRes, this, &LSPStylePrivate::handleRangeFormattingReplace);
    connect(client, &newlsp::Client::documentHighlightResult, this, &LSPStylePrivate::handleDocumentHighlight);

    /* to use QOverload cast virtual slot can't working */
    connect(client, qOverload<const newlsp::Location &, const QString &>(&newlsp::Client::definitionRes),
            this, qOverload<const newlsp::Location &, const QString &>(&LSPStylePrivate::handleCodeDefinition));
    connect(client, qOverload<const std::vector<newlsp::Location> &, const QString &>(&newlsp::Client::definitionRes),
            this, qOverload<const std::vector<newlsp::Location> &, const QString &>(&LSPStylePrivate::handleCodeDefinition));
    connect(client, qOverload<const std::vector<newlsp::LocationLink> &, const QString &>(&newlsp::Client::definitionRes),
            this, qOverload<const std::vector<newlsp::LocationLink> &, const QString &>(&LSPStylePrivate::handleCodeDefinition));

    // symbol
    auto docSymbolResult = qOverload<const QList<newlsp::DocumentSymbol> &, const QString &>(&newlsp::Client::symbolResult);
    connect(client, docSymbolResult, this, &LSPStylePrivate::handleDocumentSymbolResult);
    auto symbolInfoResult = qOverload<const QList<newlsp::SymbolInformation> &, const QString &>(&newlsp::Client::symbolResult);
    connect(client, symbolInfoResult, this, &LSPStylePrivate::handleSymbolInfomationResult);
}

void LSPStylePrivate::initIndicStyle()
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

QString LSPStylePrivate::formatDiagnosticMessage(const QString &message, int type)
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

bool LSPStylePrivate::shouldStartCompletion(const QString &insertedText)
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

int LSPStylePrivate::wordPostion()
{
    int pos = editor->cursorPosition();
    if (editor->hasSelectedText())
        return editor->wordStartPositoin(pos);

    return pos;
}

newlsp::Client *LSPStylePrivate::getClient()
{
    if (prjectKey.isValid())
        return LSPClientManager::instance()->get(prjectKey);

    auto prjSrv = dpfGetService(dpfservice::ProjectService);
    const auto &filePath = editor->getFile();
    const auto &allProject = prjSrv->getAllProjectInfo();
    for (const auto &prj : allProject) {
        const auto &files = prj.sourceFiles();
        if (!files.contains(filePath))
            continue;

        prjectKey.language = prj.language().toStdString();
        prjectKey.workspace = prj.workspaceFolder().toStdString();
        break;
    }

    if (!prjectKey.isValid()) {
        auto prj = prjSrv->getActiveProjectInfo();
        prjectKey.language = prj.language().toStdString();
        prjectKey.workspace = prj.workspaceFolder().toStdString();
    }

    auto fileLangId = support_file::Language::id(filePath);
    if (fileLangId != prjectKey.language.c_str()) {
        fileLangId = support_file::Language::idAlias(fileLangId);
        if (fileLangId != prjectKey.language.c_str())
            return nullptr;
    }

    return LSPClientManager::instance()->get(prjectKey);
}

QColor LSPStylePrivate::symbolIndicColor(lsp::SemanticTokenType::type_value token,
                                         QList<lsp::SemanticTokenType::type_index> modifier)
{
    Q_UNUSED(modifier);
    QMap<int, QColor> result;

    const auto &filePath = editor->getFile();
    auto langId = support_file::Language::id(filePath);

    return LSPClientManager::instance()->highlightColor(langId, token);
}

lsp::SemanticTokenType::type_value LSPStylePrivate::tokenToDefine(int token)
{
    auto client = getClient();
    if (!client)
        return {};
    auto initSecTokensProvider = client->initSecTokensProvider();
    if (0 <= token && token < initSecTokensProvider.legend.tokenTypes.size())
        return initSecTokensProvider.legend.tokenTypes[token];
    return {};
}

void LSPStylePrivate::handleDiagnostics(const newlsp::PublishDiagnosticsParams &data)
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

void LSPStylePrivate::cleanDiagnostics()
{
    diagnosticCache.clear();
}

void LSPStylePrivate::handleTokenFull(const QList<lsp::Data> &tokens, const QString &filePath)
{
    qInfo() << Q_FUNC_INFO << tokens.size();
    if (!editor || editor->getFile() != filePath || !editor->lexer())
        return;

    QList<std::tuple<int, QString, QString>> textTokenList;
    int cacheLine = 0;
    int cacheColumn = 0;
    for (auto val : tokens) {
        cacheLine += val.start.line;
        if (val.start.line != 0)
            cacheColumn = 0;

        cacheColumn += val.start.character;
#ifdef QT_DEBUG
        qInfo() << "line:" << cacheLine;
        qInfo() << "charStart:" << val.start.character;
        qInfo() << "charLength:" << val.length;
        qInfo() << "tokenType:" << val.tokenType;
        qInfo() << "tokenModifiers:" << val.tokenModifiers;
#endif
        auto startPos = editor->positionFromLineIndex(cacheLine, cacheColumn);
        auto wordEndPos = editor->SendScintilla(TextEditor::SCI_WORDENDPOSITION, static_cast<ulong>(startPos), true);
        auto wordStartPos = editor->SendScintilla(TextEditor::SCI_WORDSTARTPOSITION, static_cast<ulong>(startPos), true);
        if (startPos == 0 || wordEndPos == editor->length() || wordStartPos != startPos)
            continue;

        QString sourceText = editor->text(static_cast<int>(wordStartPos), static_cast<int>(wordEndPos));
#ifdef QT_DEBUG
        qInfo() << "text:" << sourceText;
#endif
        if (!sourceText.isEmpty() && sourceText.length() == val.length) {
            QString tokenValue = tokenToDefine(val.tokenType);
#ifdef QT_DEBUG
            qInfo() << "tokenValue:" << tokenValue;
#endif
            textTokenList << std::make_tuple(startPos, sourceText, tokenValue);
        }
    }

    if (textTokenList.isEmpty())
        return;

    // clear all text color
    editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_TEXTFORE);
    editor->SendScintilla(TextEditor::SCI_INDICATORCLEARRANGE, 0, editor->length());
    for (const auto &value : textTokenList) {
        auto color = symbolIndicColor(std::get<2>(value), {});
        editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_TEXTFORE);
        editor->SendScintilla(TextEditor::SCI_INDICSETFLAGS, TextEditor::INDIC_TEXTFORE, 1);
        editor->SendScintilla(TextEditor::SCI_SETINDICATORVALUE, color);
        editor->SendScintilla(TextEditor::SCI_INDICATORFILLRANGE, static_cast<ulong>(std::get<0>(value)), std::get<1>(value).length());
    }
}

void LSPStylePrivate::handleShowHoverInfo(const newlsp::Hover &hover)
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

void LSPStylePrivate::handleCodeDefinition(const newlsp::Location &data, const QString &filePath)
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

void LSPStylePrivate::handleCodeDefinition(const std::vector<newlsp::Location> &data, const QString &filePath)
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

void LSPStylePrivate::handleCodeDefinition(const std::vector<newlsp::LocationLink> &data, const QString &filePath)
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

void LSPStylePrivate::cleanDefinition(int pos)
{
    auto data = editor->SendScintilla(TextEditor::SCI_INDICATORALLONFOR, pos);
    std::bitset<32> flags(static_cast<ulong>(data));
    if (flags[TextEditor::INDIC_COMPOSITIONTHICK]) {
        editor->SendScintilla(TextEditor::SCI_SETCURSOR, definitionCache.getCursor());
        editor->SendScintilla(TextEditor::SCI_INDICATORCLEARRANGE, 0, editor->length());
    }
}

void LSPStylePrivate::handleRangeFormattingReplace(const std::vector<newlsp::TextEdit> &edits, const QString &filePath)
{
    if (edits.empty() || !editor || editor->getFile() != filePath)
        return;

    for (auto itera = edits.rbegin(); itera != edits.rend(); itera++) {
        editor->replaceRange(itera->range.start.line, itera->range.start.character,
                             itera->range.end.line, itera->range.end.character,
                             QString::fromStdString(itera->newText));
    }
}

void LSPStylePrivate::setDefinitionSelectedStyle(int start, int end)
{
    editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_COMPOSITIONTHICK);
    editor->SendScintilla(TextEditor::SCI_INDICATORFILLRANGE, static_cast<ulong>(start), end - start);

    auto cursor = editor->SendScintilla(TextEditor::SCI_GETCURSOR);
    if (cursor != 8) {
        definitionCache.setCursor(static_cast<int>(cursor));
        editor->SendScintilla(TextEditor::SCI_SETCURSOR, 8);   // hand from Scintilla platfrom.h
    }
}

void LSPStylePrivate::delayTextChanged()
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

void LSPStylePrivate::delayPositionChanged()
{
    if (!editor || !getClient())
        return;

    lsp::Position pos;
    editor->lineIndexFromPosition(editor->cursorPosition(), &pos.line, &pos.character);
    getClient()->docHighlightRequest(editor->getFile(), pos);
}

void LSPStylePrivate::handleHoveredStart(int position)
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

void LSPStylePrivate::handleHoverEnd(int position)
{
    if (!editor)
        return;

    auto textRange = hoverCache.getTextRange();
    if (!textRange.isEmpty() && !hoverCache.getTextRange().contaions(position)) {
        editor->cancelTips();
        hoverCache.clean();
    }
}

void LSPStylePrivate::handleFollowTypeStart(int position)
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

void LSPStylePrivate::handleFollowTypeEnd()
{
    if (!editor || definitionCache.getTextRange().isEmpty())
        return;

    cleanDefinition(definitionCache.getPosition());
    definitionCache.clean();
}

void LSPStylePrivate::handleIndicClicked(int line, int index)
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

void LSPStylePrivate::handleShowContextMenu(QMenu *menu)
{
    if (!editor)
        return;

    auto actionList = menu->actions();
    for (auto act : actionList) {
        if (act->text() == tr("Refactor")) {
            QMenu *subMenu = new QMenu(menu);
            subMenu->addAction(tr("Rename Symbol Under Cursor"), q, &LSPStyle::renameActionTriggered);
            act->setMenu(subMenu);
            break;
        }
    }

    auto act = menu->addAction(tr("Switch Header/Source"), q, std::bind(&LSPStyle::switchHeaderSource, q, editor->getFile()));
    menu->insertAction(actionList.first(), act);

    act = menu->addAction(tr("Follow Symbol Under Cursor"), q, &LSPStyle::followSymbolUnderCursor);
    menu->insertAction(actionList.first(), act);

    act = menu->addAction(tr("Find Usages"), q, &LSPStyle::findUsagesActionTriggered);
    menu->insertAction(actionList.first(), act);

    act = menu->addAction(tr("Range Formatting"), q, &LSPStyle::formatSelections);
    menu->insertAction(actionList.first(), act);
    menu->insertSeparator(actionList.first());
}

void LSPStylePrivate::handleFileClosed(const QString &file)
{
    if (getClient())
        getClient()->closeRequest(file);
}

void LSPStylePrivate::handleRename(const QString &text)
{
    if (!editor || !getClient() || !renameCache.isValid())
        return;

    lsp::Position pos { renameCache.line, renameCache.column };
    getClient()->renameRequest(editor->getFile(), pos, text);
    renameCache.clear();
}

void LSPStylePrivate::gotoDefinition()
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

void LSPStylePrivate::handleSwitchHeaderSource(const QString &file)
{
    if (file.isEmpty())
        return;

    emit EditorCallProxy::instance()->reqOpenFile("", file);
}

void LSPStylePrivate::handleDocumentSymbolResult(const QList<newlsp::DocumentSymbol> &docSymbols,
                                                 const QString &filePath)
{
    if (!editor || editor->getFile() != filePath)
        return;

    docSymbolList = docSymbols;
}

void LSPStylePrivate::handleSymbolInfomationResult(const QList<newlsp::SymbolInformation> &symbolInfos,
                                                   const QString &filePath)
{
    if (!editor || editor->getFile() != filePath)
        return;

    symbolInfoList = symbolInfos;
}

void LSPStylePrivate::handleDocumentHighlight(const QList<newlsp::DocumentHighlight> &docHighlightList,
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

LSPStyle::LSPStyle(TextEditor *parent)
    : QObject(parent),
      d(new LSPStylePrivate(parent, this))
{
    d->initConnection();
    d->initLspConnection();
    d->initIndicStyle();
}

LSPStyle::~LSPStyle()
{
}

void LSPStyle::requestCompletion(int line, int column)
{
    if (!d->getClient())
        return;

    lsp::Position pos { line, column };
    d->getClient()->completionRequest(d->editor->getFile(), pos);
}

void LSPStyle::updateTokens()
{
    if (auto client = d->getClient()) {
        client->openRequest(d->editor->getFile());
        client->docSemanticTokensFull(d->editor->getFile());
        client->symbolRequest(d->editor->getFile());
    }
}

QList<newlsp::DocumentSymbol> LSPStyle::documentSymbolList() const
{
    return d->docSymbolList;
}

QList<newlsp::SymbolInformation> LSPStyle::symbolInformationList() const
{
    return d->symbolInfoList;
}

void LSPStyle::refreshTokens()
{
    if (!d->editor || !d->getClient())
        return;

    d->getClient()->docSemanticTokensFull(d->editor->getFile());
}

void LSPStyle::switchHeaderSource(const QString &file)
{
    if (!d->getClient())
        return;

    d->getClient()->switchHeaderSource(file);
}

void LSPStyle::followSymbolUnderCursor()
{
    if (!d->editor || !d->editor->hasFocus() || !d->getClient())
        return;

    d->definitionCache.setSwitchMode(DefinitionCache::ActionMode);

    lsp::Position pos;
    d->editor->lineIndexFromPosition(d->wordPostion(), &pos.line, &pos.character);
    d->getClient()->definitionRequest(d->editor->getFile(), pos);
}

void LSPStyle::findUsagesActionTriggered()
{
    if (!d->editor || !d->getClient())
        return;

    lsp::Position pos;
    d->editor->lineIndexFromPosition(d->wordPostion(), &pos.line, &pos.character);
    d->getClient()->referencesRequest(d->editor->getFile(), pos);
}

void LSPStyle::renameActionTriggered()
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

void LSPStyle::formatSelections()
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
