// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lspstyle.h"
#include "private/lspstyle_p.h"
#include "gui/texteditor.h"
#include "lspclientmanager.h"
#include "transceiver/codeeditorreceiver.h"
#include "utils/colordefine.h"
#include "codelens/codelens.h"

#include "services/project/projectservice.h"

#include "Qsci/qscilexer.h"

#include <DApplicationHelper>
#include <QApplication>

#include <bitset>

DGUI_USE_NAMESPACE

LSPStyle::LSPStyle(TextEditor *parent)
    : QObject(parent),
      d(new LSPStylePrivate)
{
    d->editor = parent;
    d->diagnosticFormat = "%1\n%2:%3";
    d->textChangedTimer.setSingleShot(true);
    d->textChangedTimer.setInterval(200);

    setIndicStyle();

    connect(d->editor, &TextEditor::textChanged, this, [this] { d->textChangedTimer.start(); });
    connect(d->editor, &TextEditor::documentHovered, this, &LSPStyle::onHovered);
    connect(d->editor, &TextEditor::documentHoverEnd, this, &LSPStyle::onHoverCleaned);
    connect(d->editor, &TextEditor::documentHoveredWithCtrl, this, &LSPStyle::onDefinitionHover);
    connect(d->editor, &TextEditor::indicatorClicked, this, &LSPStyle::onIndicClicked);
    connect(d->editor, &TextEditor::contextMenuRequested, this, &LSPStyle::onShowContextMenu);
    connect(d->editor, &TextEditor::fileClosed, this, &LSPStyle::onFileClosed);
    connect(&d->renamePopup, &RenamePopup::editingFinished, this, &LSPStyle::renameSymbol);
    connect(&d->textChangedTimer, &QTimer::timeout, this, &LSPStyle::onTextChanged);
    connect(CodeLens::instance(), &CodeLens::doubleClicked,
            this, [=](const QString &filePath, const lsp::Range &range) {
                emit EditorCallProxy::instance()->reqGotoPosition(filePath, range.start.line, range.start.character);
            });
    connect(qApp, &QApplication::applicationStateChanged, this, [=](Qt::ApplicationState state) {
        if (state == Qt::ApplicationState::ApplicationInactive)
            d->editor->cancelTips();
    });
}

LSPStyle::~LSPStyle()
{
}

void LSPStyle::initLspConnection()
{
    if (!d->editor)
        return;

    connect(d->getClient(), QOverload<const lsp::References &>::of(&newlsp::Client::requestResult),
            CodeLens::instance(), &CodeLens::displayReference, Qt::UniqueConnection);
    connect(d->getClient(), &newlsp::Client::switchHeaderSourceResult,
            this, &LSPStyle::handleSwitchHeaderSource, Qt::UniqueConnection);

    //bind signals to file diagnostics
    connect(d->getClient(), QOverload<const newlsp::PublishDiagnosticsParams &>::of(&newlsp::Client::publishDiagnostics),
            this, [=](const newlsp::PublishDiagnosticsParams &data) { this->setDiagnostics(data); });

    connect(d->getClient(), QOverload<const QList<lsp::Data> &, const QString &>::of(&newlsp::Client::requestResult),
            this, &LSPStyle::setTokenFull);

    connect(d->getClient(), QOverload<const newlsp::Hover &>::of(&newlsp::Client::hoverRes),
            this, &LSPStyle::setHover);

    connect(d->getClient(), QOverload<const lsp::CompletionProvider &>::of(&newlsp::Client::requestResult),
            this, &LSPStyle::completeFinished);

    connect(d->getClient(), QOverload<const newlsp::WorkspaceEdit &>::of(&newlsp::Client::renameRes),
            EditorCallProxy::instance(), &EditorCallProxy::reqDoRename, Qt::UniqueConnection);

    /* to use QOverload cast virtual slot can't working */
    connect(d->getClient(), QOverload<const newlsp::Location &, const QString &>::of(&newlsp::Client::definitionRes),
            this, QOverload<const newlsp::Location &, const QString &>::of(&LSPStyle::setDefinition));
    connect(d->getClient(), QOverload<const std::vector<newlsp::Location> &, const QString &>::of(&newlsp::Client::definitionRes),
            this, QOverload<const std::vector<newlsp::Location> &, const QString &>::of(&LSPStyle::setDefinition));
    connect(d->getClient(), QOverload<const std::vector<newlsp::LocationLink> &, const QString &>::of(&newlsp::Client::definitionRes),
            this, QOverload<const std::vector<newlsp::LocationLink> &, const QString &>::of(&LSPStyle::setDefinition));
}

void LSPStyle::requestCompletion(int line, int column)
{
    if (!d->getClient())
        return;

    lsp::Position pos { line, column };
    qApp->metaObject()->invokeMethod(d->getClient(), "completionRequest",
                                     Q_ARG(const QString &, d->editor->getFile()),
                                     Q_ARG(const lsp::Position &, pos));
}

void LSPStyle::updateTokens()
{
    if (d->getClient()) {
        qApp->metaObject()->invokeMethod(d->getClient(), "openRequest", Qt::QueuedConnection, Q_ARG(const QString &, d->editor->getFile()));
        qApp->metaObject()->invokeMethod(d->getClient(), "docSemanticTokensFull", Qt::QueuedConnection, Q_ARG(const QString &, d->editor->getFile()));
    }
}

QColor LSPStyle::symbolIndicColor(lsp::SemanticTokenType::type_value token,
                                  QList<lsp::SemanticTokenType::type_index> modifier)
{
    Q_UNUSED(modifier);
    QMap<int, QColor> result;

    const auto &filePath = d->editor->getFile();
    auto langId = support_file::Language::id(filePath);

    return LSPClientManager::instance()->highlightColor(langId, token);
}

lsp::SemanticTokenType::type_value LSPStyle::tokenToDefine(int token)
{
    auto client = d->getClient();
    if (!client)
        return {};
    auto initSecTokensProvider = client->initSecTokensProvider();
    if (0 <= token && token < initSecTokensProvider.legend.tokenTypes.size())
        return initSecTokensProvider.legend.tokenTypes[token];
    return {};
}

void LSPStyle::setIndicStyle()
{
    d->editor->indicatorDefine(TextEditor::PlainIndicator, TextEditor::INDIC_PLAIN);
    d->editor->indicatorDefine(TextEditor::SquiggleIndicator, TextEditor::INDIC_SQUIGGLE);
    d->editor->indicatorDefine(TextEditor::TTIndicator, TextEditor::INDIC_TT);
    d->editor->indicatorDefine(TextEditor::DiagonalIndicator, TextEditor::INDIC_DIAGONAL);
    d->editor->indicatorDefine(TextEditor::StrikeIndicator, TextEditor::INDIC_STRIKE);
    d->editor->indicatorDefine(TextEditor::HiddenIndicator, TextEditor::INDIC_HIDDEN);
    d->editor->indicatorDefine(TextEditor::BoxIndicator, TextEditor::INDIC_BOX);
    d->editor->indicatorDefine(TextEditor::RoundBoxIndicator, TextEditor::INDIC_ROUNDBOX);
    d->editor->indicatorDefine(TextEditor::StraightBoxIndicator, TextEditor::INDIC_STRAIGHTBOX);
    d->editor->indicatorDefine(TextEditor::FullBoxIndicator, TextEditor::INDIC_FULLBOX);
    d->editor->indicatorDefine(TextEditor::DashesIndicator, TextEditor::INDIC_DASH);
    d->editor->indicatorDefine(TextEditor::DotsIndicator, TextEditor::INDIC_DOTS);
    d->editor->indicatorDefine(TextEditor::SquiggleLowIndicator, TextEditor::INDIC_SQUIGGLELOW);
    d->editor->indicatorDefine(TextEditor::DotBoxIndicator, TextEditor::INDIC_DOTBOX);
    d->editor->indicatorDefine(TextEditor::GradientIndicator, TextEditor::INDIC_GRADIENT);
    d->editor->indicatorDefine(TextEditor::GradientIndicator, TextEditor::INDIC_GRADIENTCENTRE);
    d->editor->indicatorDefine(TextEditor::SquigglePixmapIndicator, TextEditor::INDIC_SQUIGGLEPIXMAP);
    d->editor->indicatorDefine(TextEditor::ThickCompositionIndicator, TextEditor::INDIC_COMPOSITIONTHICK);
    d->editor->indicatorDefine(TextEditor::ThinCompositionIndicator, TextEditor::INDIC_COMPOSITIONTHIN);
    d->editor->indicatorDefine(TextEditor::TextColorIndicator, TextEditor::INDIC_TEXTFORE);
    d->editor->indicatorDefine(TextEditor::TriangleIndicator, TextEditor::INDIC_POINT);
    d->editor->indicatorDefine(TextEditor::TriangleCharacterIndicator, TextEditor::INDIC_POINTCHARACTER);
}

void LSPStyle::setMargin()
{
}

void LSPStyle::setDiagnostics(const newlsp::PublishDiagnosticsParams &data)
{
    if (!d->editor)
        return;

    if (QUrl(QString::fromStdString(data.uri)).toLocalFile() != d->editor->getFile())
        return;

    // clear all flags of diagnostics
    d->editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_SQUIGGLE);
    d->editor->SendScintilla(TextEditor::SCI_INDICATORCLEARRANGE, 0, d->editor->length());
    this->cleanDiagnostics();
    for (auto val : data.diagnostics) {
        if (newlsp::Enum::DiagnosticSeverity::get()->Error == val.severity.value()) {   // error
            newlsp::Position start { val.range.start.line, val.range.start.character };
            newlsp::Position end { val.range.end.line, val.range.end.character };
            int startPos = d->editor->positionFromLineIndex(start.line, start.character);
            int endPos = d->editor->positionFromLineIndex(end.line, end.character);

            d->editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_SQUIGGLE);
            d->editor->SendScintilla(TextEditor::SCI_INDICSETFORE, TextEditor::INDIC_SQUIGGLE, QColor(Qt::red));
            d->editor->SendScintilla(TextEditor::SCI_INDICATORFILLRANGE, static_cast<ulong>(startPos), endPos - startPos);

            std::string message = val.message.value();
            d->diagnosticCache.append({ startPos, endPos, message.c_str(), AnnotationType::ErrorAnnotation });
        }
    }
}

void LSPStyle::cleanDiagnostics()
{
    d->diagnosticCache.clear();
}

void LSPStyle::setTokenFull(const QList<lsp::Data> &tokens, const QString &filePath)
{
    qInfo() << Q_FUNC_INFO << tokens.size();
    if (!d->editor || d->editor->getFile() != filePath || !d->editor->lexer())
        return;

    QList<std::tuple<int, QString, QString>> textTokenList;
    int cacheLine = 0;
    for (auto val : tokens) {
        cacheLine += val.start.line;
#ifdef QT_DEBUG
        qInfo() << "line:" << cacheLine;
        qInfo() << "charStart:" << val.start.character;
        qInfo() << "charLength:" << val.length;
        qInfo() << "tokenType:" << val.tokenType;
        qInfo() << "tokenModifiers:" << val.tokenModifiers;
#endif
        auto startPos = d->editor->positionFromLineIndex(cacheLine, val.start.character);
        auto wordEndPos = d->editor->SendScintilla(TextEditor::SCI_WORDENDPOSITION, static_cast<ulong>(startPos), true);
        auto wordStartPos = d->editor->SendScintilla(TextEditor::SCI_WORDSTARTPOSITION, static_cast<ulong>(startPos), true);
        if (startPos == 0 || wordEndPos == d->editor->length() || wordStartPos != startPos)
            continue;

        QString sourceText = d->editor->text(static_cast<int>(wordStartPos), static_cast<int>(wordEndPos));
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
    d->editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_TEXTFORE);
    d->editor->SendScintilla(TextEditor::SCI_INDICATORCLEARRANGE, 0, d->editor->length());
    for (const auto &value : textTokenList) {
        auto color = symbolIndicColor(std::get<2>(value), {});
        d->editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_TEXTFORE);
        d->editor->SendScintilla(TextEditor::SCI_INDICSETFLAGS, TextEditor::INDIC_TEXTFORE, 1);
        d->editor->SendScintilla(TextEditor::SCI_SETINDICATORVALUE, color);
        d->editor->SendScintilla(TextEditor::SCI_INDICATORFILLRANGE, static_cast<ulong>(std::get<0>(value)), std::get<1>(value).length());
    }

    //            QString tokenAnnLine = TextEditKeeper::getTokenTypeAnnLine(tokenValue, sourceText);
    //            if (!tokenAnnLine.isEmpty()) {
    //                editor.setAnnotation(d->edit->file(), cacheLine,
    //                                     QString(TextEditKeeper::userActionAnalyseTitle()),
    //                                     AnnotationInfo { tokenAnnLine });
    //            }
}

void LSPStyle::refreshTokens()
{
    if (!d->editor || !d->getClient())
        return;

    qApp->metaObject()->invokeMethod(d->getClient(),
                                     "docSemanticTokensFull",
                                     Qt::QueuedConnection,
                                     Q_ARG(const QString &, d->editor->getFile()));
}

void LSPStyle::cleanTokenFull()
{
}

void LSPStyle::setHover(const newlsp::Hover &hover)
{
    if (!d->editor || d->hoverCache.getPosition() == -1)
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
        d->editor->showTips(d->hoverCache.getPosition(), showText.c_str());
}

void LSPStyle::setDefinition(const newlsp::Location &data, const QString &filePath)
{
    if (!d->editor || d->editor->getFile() != filePath)
        return;

    d->definitionCache.set(data);
    if (d->definitionCache.switchMode() == DefinitionCache::ClickMode) {
        auto textRange = d->definitionCache.getTextRange();
        setDefinitionSelectedStyle(textRange.getStart(), textRange.getEnd());
    } else {
        gotoDefinition();
        d->definitionCache.clean();
    }
}

void LSPStyle::setDefinition(const std::vector<newlsp::Location> &data, const QString &filePath)
{
    if (!d->editor || data.empty() || d->editor->getFile() != filePath)
        return;

    d->definitionCache.set(data);
    if (d->definitionCache.switchMode() == DefinitionCache::ClickMode) {
        auto textRange = d->definitionCache.getTextRange();
        if (!textRange.isEmpty())
            setDefinitionSelectedStyle(textRange.getStart(), textRange.getEnd());
    } else {
        gotoDefinition();
        d->definitionCache.clean();
    }
}

void LSPStyle::setDefinition(const std::vector<newlsp::LocationLink> &data, const QString &filePath)
{
    if (!d->editor || data.empty() || d->editor->getFile() != filePath)
        return;

    d->definitionCache.set(data);
    if (d->definitionCache.switchMode() == DefinitionCache::ClickMode) {
        auto textRange = d->definitionCache.getTextRange();
        setDefinitionSelectedStyle(textRange.getStart(), textRange.getEnd());
    } else {
        gotoDefinition();
        d->definitionCache.clean();
    }
}

void LSPStyle::cleanDefinition(int pos)
{
    auto data = d->editor->SendScintilla(TextEditor::SCI_INDICATORALLONFOR, pos);
    std::bitset<32> flags(static_cast<ulong>(data));
    if (flags[TextEditor::INDIC_COMPOSITIONTHICK]) {
        d->editor->SendScintilla(TextEditor::SCI_SETCURSOR, d->definitionCache.getCursor());
        d->editor->SendScintilla(TextEditor::SCI_INDICATORCLEARRANGE, 0, d->editor->length());
    }
}

void LSPStyle::switchHeaderSource(const QString &file)
{
    if (!d->getClient())
        return;

    qApp->metaObject()->invokeMethod(d->getClient(), "switchHeaderSource",
                                     Q_ARG(const QString &, file));
}

void LSPStyle::setDefinitionSelectedStyle(int start, int end)
{
    d->editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, TextEditor::INDIC_COMPOSITIONTHICK);
    d->editor->SendScintilla(TextEditor::SCI_INDICSETFORE, d->editor->SendScintilla(TextEditor::SCI_STYLEGETFORE, 0));
    d->editor->SendScintilla(TextEditor::SCI_INDICATORFILLRANGE, static_cast<ulong>(start), end - start);

    auto cursor = d->editor->SendScintilla(TextEditor::SCI_GETCURSOR);
    if (cursor != 8) {
        d->definitionCache.setCursor(static_cast<int>(cursor));
        d->editor->SendScintilla(TextEditor::SCI_SETCURSOR, 8);   // hand from Scintilla platfrom.h
    }
}

void LSPStyle::onTextChanged()
{
    if (!d->editor)
        return;

    cleanDiagnostics();
    if (d->getClient()) {
        const auto &content = d->editor->text();
        qApp->metaObject()->invokeMethod(d->getClient(), "changeRequest",
                                         Q_ARG(const QString &, d->editor->getFile()),
                                         Q_ARG(const QByteArray &, content.toUtf8()));
        qApp->metaObject()->invokeMethod(d->getClient(), "docSemanticTokensFull",
                                         Q_ARG(const QString &, d->editor->getFile()));
    }
}

void LSPStyle::onHovered(int position)
{
    if (!d->editor || !d->getClient())
        return;

    if (!d->diagnosticCache.isEmpty()) {
        auto iter = std::find_if(d->diagnosticCache.begin(), d->diagnosticCache.end(),
                                 [position](const DiagnosticCache &cache) {
                                     return cache.contains(position);
                                 });
        if (iter != d->diagnosticCache.end()) {
            const auto &msg = d->formatDiagnosticMessage(iter->message, iter->type);
            d->editor->showTips(position, msg);
            return;
        }
    }

    d->hoverCache.setPosition(position);
    auto textRange = d->hoverCache.getTextRange();
    if (!textRange.isEmpty() && textRange.contaions(position))
        return;

    auto startPos = d->editor->SendScintilla(TextEditor::SCI_WORDSTARTPOSITION, static_cast<ulong>(position), true);
    auto endPos = d->editor->SendScintilla(TextEditor::SCI_WORDENDPOSITION, static_cast<ulong>(position), true);
    // hover in the empty area
    if (startPos == endPos)
        return;

    d->hoverCache.setTextRange(static_cast<int>(startPos), static_cast<int>(endPos));
    lsp::Position pos;
    d->editor->lineIndexFromPosition(position, &pos.line, &pos.character);
    qApp->metaObject()->invokeMethod(d->getClient(), "docHoverRequest",
                                     Q_ARG(const QString &, d->editor->getFile()),
                                     Q_ARG(const lsp::Position &, pos));
}

void LSPStyle::onHoverCleaned(int position)
{
    if (!d->editor)
        return;

    auto textRange = d->hoverCache.getTextRange();
    if (!textRange.isEmpty() && !d->hoverCache.getTextRange().contaions(position)) {
        d->editor->cancelTips();
        d->hoverCache.clean();
    }
    onDefinitionHoverCleaned(position);
}

void LSPStyle::onDefinitionHover(int position)
{
    if (!d->editor)
        return;

    auto startPos = d->editor->SendScintilla(TextEditor::SCI_WORDSTARTPOSITION, static_cast<ulong>(position), true);
    auto endPos = d->editor->SendScintilla(TextEditor::SCI_WORDENDPOSITION, static_cast<ulong>(position), true);
    RangeCache textRange { static_cast<int>(startPos), static_cast<int>(endPos) };

    if (d->definitionCache.getTextRange() == textRange)
        return;

    if (!d->definitionCache.getTextRange().isEmpty())
        onDefinitionHoverCleaned(position);

    d->definitionCache.setPosition(position);
    d->definitionCache.setTextRange(textRange);
    d->definitionCache.cleanFromLsp();
    d->definitionCache.setSwitchMode(DefinitionCache::ClickMode);

    lsp::Position pos;
    d->editor->lineIndexFromPosition(position, &pos.line, &pos.character);
    if (d->getClient()) {
        qApp->metaObject()->invokeMethod(d->getClient(), "definitionRequest",
                                         Q_ARG(const QString &, d->editor->getFile()),
                                         Q_ARG(const lsp::Position &, pos));
    }
}

void LSPStyle::onDefinitionHoverCleaned(int position)
{
    if (!d->editor || d->definitionCache.getTextRange().isEmpty())
        return;

    auto startPos = d->editor->SendScintilla(TextEditor::SCI_WORDSTARTPOSITION, static_cast<ulong>(position), true);
    auto endPos = d->editor->SendScintilla(TextEditor::SCI_WORDENDPOSITION, static_cast<ulong>(position), true);
    RangeCache textRange { static_cast<int>(startPos), static_cast<int>(endPos) };
    bool isKeyCtrl = QApplication::keyboardModifiers().testFlag(Qt::ControlModifier);
    bool isSameRange = d->definitionCache.getTextRange() == textRange;

    if (!isSameRange || !isKeyCtrl) {
        cleanDefinition(d->definitionCache.getPosition());
        d->definitionCache.clean();
    }
}

void LSPStyle::onIndicClicked(int line, int index)
{
    if (!d->editor)
        return;

    auto pos = d->editor->positionFromLineIndex(line, index);
    auto data = d->editor->SendScintilla(TextEditor::SCI_INDICATORALLONFOR, pos);
    std::bitset<32> flags(static_cast<ulong>(data));
    if (flags[TextEditor::INDIC_COMPOSITIONTHICK]) {
        gotoDefinition();
        cleanDefinition(pos);
    }
}

void LSPStyle::onShowContextMenu(QMenu *menu)
{
    if (!d->editor)
        return;

    auto actionList = menu->actions();
    for (auto act : actionList) {
        if (act->text() == tr("Refactor")) {
            QMenu *subMenu = new QMenu(menu);
            subMenu->addAction(tr("Rename Symbol Under Cursor"), this, &LSPStyle::renameActionTriggered);
            act->setMenu(subMenu);
            break;
        }
    }

    auto act = menu->addAction(tr("Switch Header/Source"), this, std::bind(&LSPStyle::switchHeaderSource, this, d->editor->getFile()));
    menu->insertAction(actionList.first(), act);

    act = menu->addAction(tr("Follow Symbol Under Cursor"), this, &LSPStyle::followSymbolUnderCursor);
    menu->insertAction(actionList.first(), act);

    act = menu->addAction(tr("Find Usages"), this, &LSPStyle::findUsagesActionTriggered);
    menu->insertAction(actionList.first(), act);
    menu->insertSeparator(actionList.first());
}

void LSPStyle::onFileClosed(const QString &file)
{
    if (d->getClient())
        qApp->metaObject()->invokeMethod(d->getClient(), "closeRequest", Q_ARG(const QString &, file));
}

void LSPStyle::followSymbolUnderCursor()
{
    if (!d->editor || !d->editor->hasFocus() || !d->getClient())
        return;

    d->definitionCache.setSwitchMode(DefinitionCache::ActionMode);

    lsp::Position pos;
    d->editor->lineIndexFromPosition(d->editor->cursorPosition(), &pos.line, &pos.character);
    qApp->metaObject()->invokeMethod(d->getClient(), "definitionRequest",
                                     Q_ARG(const QString &, d->editor->getFile()),
                                     Q_ARG(const lsp::Position &, pos));
}

void LSPStyle::findUsagesActionTriggered()
{
    if (!d->editor || !d->getClient())
        return;

    lsp::Position pos;
    d->editor->lineIndexFromPosition(d->editor->cursorPosition(), &pos.line, &pos.character);
    qApp->metaObject()->invokeMethod(d->getClient(), "referencesRequest",
                                     Q_ARG(const QString &, d->editor->getFile()),
                                     Q_ARG(const lsp::Position &, pos));
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

void LSPStyle::renameSymbol(const QString &text)
{
    if (!d->editor || !d->getClient() || !d->renameCache.isValid())
        return;

    lsp::Position pos { d->renameCache.line, d->renameCache.column };
    qApp->metaObject()->invokeMethod(d->getClient(), "renameRequest",
                                     Q_ARG(const QString &, d->editor->getFile()),
                                     Q_ARG(const lsp::Position &, pos),
                                     Q_ARG(const QString &, text));
    d->renameCache.clear();
}

void LSPStyle::gotoDefinition()
{
    if (d->definitionCache.getLocations().size() > 0) {
        auto one = d->definitionCache.getLocations().front();
        EditorCallProxy::instance()->reqGotoPosition(QUrl(QString::fromStdString(one.uri)).toLocalFile(),
                                                     one.range.start.line, one.range.start.character);
    } else if (d->definitionCache.getLocationLinks().size() > 0) {
        auto one = d->definitionCache.getLocationLinks().front();
        EditorCallProxy::instance()->reqGotoPosition(QUrl(QString::fromStdString(one.targetUri)).toLocalFile(),
                                                     one.targetRange.end.line, one.targetRange.end.character);
    } else {
        auto one = d->definitionCache.getLocation();
        EditorCallProxy::instance()->reqGotoPosition(QUrl(QString::fromStdString(one.uri)).toLocalFile(),
                                                     one.range.start.line, one.range.start.character);
    }
}

void LSPStyle::handleSwitchHeaderSource(const QString &file)
{
    if (file.isEmpty())
        return;

    emit EditorCallProxy::instance()->reqOpenFile("", file);
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

newlsp::Client *LSPStylePrivate::getClient() const
{
    auto activeProjInfo = dpfGetService(dpfservice::ProjectService)->getActiveProjectInfo();
    auto langId = activeProjInfo.language();
    auto workspaceFolder = activeProjInfo.workspaceFolder();

    newlsp::ProjectKey key(langId.toStdString(), workspaceFolder.toStdString());
    const auto &filePath = editor->getFile();
    auto fileLangId = support_file::Language::id(filePath);
    if (fileLangId != langId) {
        fileLangId = support_file::Language::idAlias(fileLangId);
        if (fileLangId != langId)
            return nullptr;
    }

    return LSPClientManager::instance()->get(key);
}
