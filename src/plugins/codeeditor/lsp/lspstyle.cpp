// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lspstyle.h"
#include "private/lspstyle_p.h"
#include "gui/texteditor.h"
#include "lspclientmanager.h"

#include "services/project/projectservice.h"

LSPStyle::LSPStyle(TextEditor *parent)
    : QObject(parent),
      d(new LSPStylePrivate)
{
    d->editor = parent;

    setIndicStyle();

    connect(d->editor, &TextEditor::textAdded, this, &LSPStyle::onTextInsertedTotal);
    connect(d->editor, &TextEditor::textRemoved, this, &LSPStyle::onTextDeletedTotal);
    //    connect(d->editor, &TextEditor::hovered, this, &LSPStyle::sciHovered);
    //    connect(d->editor, &TextEditor::hoverCleaned, this, &LSPStyle::sciHoverCleaned);
    //    connect(d->editor, &TextEditor::definitionHover, this, &LSPStyle::sciDefinitionHover);
    //    connect(d->editor, &TextEditor::definitionHoverCleaned, this, &LSPStyle::sciDefinitionHoverCleaned);
    //    connect(d->editor, &TextEditor::indicClicked, this, &LSPStyle::sciIndicClicked);
    //    connect(d->editor, &TextEditor::indicReleased, this, &LSPStyle::sciIndicReleased);
    //    connect(d->editor, &TextEditor::selectionMenu, this, &LSPStyle::sciSelectionMenu);
    //    connect(d->editor, &TextEditor::replaceed, this, &LSPStyle::sciReplaced);
    //    connect(d->editor, &TextEditor::fileClosed, this, &LSPStyle::sciClosed);
    //    connect(qApp, &QApplication::applicationStateChanged, this, [=](Qt::ApplicationState state) {
    //        if (state == Qt::ApplicationState::ApplicationInactive && d->edit->callTipActive())
    //            d->edit->callTipCancel();
    //    });
}

TextEditor *LSPStyle::editor()
{
}

LSPStyle::~LSPStyle()
{
}

void LSPStyle::initLspConnection()
{
    //bind signals to file diagnostics
    connect(d->getClient(), QOverload<const newlsp::PublishDiagnosticsParams &>::of(&newlsp::Client::publishDiagnostics),
            this, [=](const newlsp::PublishDiagnosticsParams &data) { this->setDiagnostics(data); });

    connect(d->getClient(), QOverload<const QList<lsp::Data> &>::of(&newlsp::Client::requestResult),
            this, &LSPStyle::setTokenFull);

    connect(d->getClient(), QOverload<const newlsp::Hover &>::of(&newlsp::Client::hoverRes),
            this, &LSPStyle::setHover);

    connect(d->getClient(), QOverload<const lsp::CompletionProvider &>::of(&newlsp::Client::requestResult),
            this, [=](const lsp::CompletionProvider &provider) {
                d->completionCache.provider = provider;
            });

    connect(d->getClient(), &newlsp::Client::rangeFormattingRes,
            this, &LSPStyle::rangeFormattingReplace);

    /* to use QOverload cast virtual slot can't working */
    connect(d->getClient(), QOverload<const newlsp::Location &>::of(&newlsp::Client::definitionRes),
            this, QOverload<const newlsp::Location &>::of(&LSPStyle::setDefinition));
    connect(d->getClient(), QOverload<const std::vector<newlsp::Location> &>::of(&newlsp::Client::definitionRes),
            this, QOverload<const std::vector<newlsp::Location> &>::of(&LSPStyle::setDefinition));
    connect(d->getClient(), QOverload<const std::vector<newlsp::LocationLink> &>::of(&newlsp::Client::definitionRes),
            this, QOverload<const std::vector<newlsp::LocationLink> &>::of(&LSPStyle::setDefinition));
}

void LSPStyle::updateTokens()
{
    if (d->getClient()) {
        qApp->metaObject()->invokeMethod(d->getClient(), "openRequest", Qt::QueuedConnection, Q_ARG(const QString &, d->editor->getFile()));
        qApp->metaObject()->invokeMethod(d->getClient(), "docSemanticTokensFull", Qt::QueuedConnection, Q_ARG(const QString &, d->editor->getFile()));
    }
}

QMap<int, QColor> LSPStyle::symbolIndic(lsp::SemanticTokenType::type_value token,
                                     QList<lsp::SemanticTokenType::type_index> modifier)
{
    Q_UNUSED(modifier);
    QMap<int, QColor> result;

    const auto &filePath = d->editor->getFile();
    auto langId = support_file::Language::id(filePath);
    result[TextEditor::INDIC_TEXTFORE] = LSPClientManager::instance()->highlightColor(langId, token);

    return result;
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
}

void LSPStyle::cleanDiagnostics()
{
}

void LSPStyle::setTokenFull(const QList<lsp::Data> &tokens)
{
    qInfo() << Q_FUNC_INFO << tokens.size();
    if (!d->editor || !d->editor->lexer())
        return;

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
            auto indics = symbolIndic(tokenValue, val.tokenModifiers);
            for (int i = 0; i < TextEditor::INDIC_MAX; i++) {
                if (indics.contains(i)) {
                    d->editor->SendScintilla(TextEditor::SCI_SETINDICATORCURRENT, i);
                    d->editor->SendScintilla(TextEditor::SCI_INDICSETFLAGS, i, 1);
                    d->editor->SendScintilla(TextEditor::SCI_SETINDICATORVALUE, indics[i]);
                    d->editor->SendScintilla(TextEditor::SCI_INDICATORFILLRANGE, startPos, sourceText.length());
                }
            }

            //            QString tokenAnnLine = TextEditKeeper::getTokenTypeAnnLine(tokenValue, sourceText);
            //            if (!tokenAnnLine.isEmpty()) {
            //                editor.setAnnotation(d->edit->file(), cacheLine,
            //                                     QString(TextEditKeeper::userActionAnalyseTitle()),
            //                                     AnnotationInfo { tokenAnnLine });
            //            }
        }
    }
}

void LSPStyle::cleanTokenFull()
{
}

void LSPStyle::setHover(const newlsp::Hover &hover)
{
}

void LSPStyle::cleanHover()
{
}

void LSPStyle::setDefinition(const newlsp::Location &data)
{
}

void LSPStyle::setDefinition(const std::vector<newlsp::Location> &data)
{
}

void LSPStyle::setDefinition(const std::vector<newlsp::LocationLink> &data)
{
}

void LSPStyle::cleanDefinition(int pos)
{
}

void LSPStyle::rangeFormattingReplace(const std::vector<newlsp::TextEdit> &edits)
{
}

bool LSPStyle::isCharSymbol(const char ch)
{
}

void LSPStyle::setDefinitionSelectedStyle(int start, int end)
{
}

void LSPStyle::setCompletion(const QString &text, int enterLenght, const lsp::CompletionProvider &provider)
{
    if (!d->editor->hasFocus())
        return;

    if (provider.items.isEmpty() || d->textChangedCache.textCache.isEmpty())
        return;

    const unsigned char sep = 0x7C;   // "|"
    d->editor->SendScintilla(TextEditor::SCI_AUTOCSETSEPARATOR, sep);
    QStringList inserts;
    for (auto item : provider.items) {
        if (!item.insertText.startsWith(text, Qt::CaseInsensitive))
            continue;
        inserts << item.insertText;
    }

    if (inserts.isEmpty())
        return;

    qSort(inserts.begin(), inserts.end(),
          [](const QString &str1, const QString &str2) {
              return str1.toLower() < str2.toLower();
          });

    auto completionData = inserts.join(sep).toUtf8();
    d->editor->SendScintilla(TextEditor::SCI_AUTOCSHOW, enterLenght, completionData.constData());
}

void LSPStyle::onTextInsertedTotal(int position, int length, int linesAdded, const QString &text, int line)
{
    Q_UNUSED(linesAdded)
    Q_UNUSED(line)

    if (!d->editor || !d->getClient())
        return;

    if (d->textChangedTimer.isActive())
        d->textChangedTimer.stop();

    auto wordStartPos = d->editor->SendScintilla(TextEditor::SCI_WORDSTARTPOSITION, static_cast<ulong>(position - length), true);
    setCompletion(d->editor->text(wordStartPos, position).replace(" ", "") + text,
                  position - wordStartPos, d->completionCache.provider);

    d->textChangedCache.positionCache = position;
    d->textChangedCache.lengthCache = length;
    d->textChangedCache.textCache = text;
    d->textChangedCache.state = TextChangeCache::State::Inserted;

    d->textChangedTimer.start(500);
    connect(&d->textChangedTimer, &QTimer::timeout,
            this, &LSPStyle::onTextChangedTotal,
            Qt::UniqueConnection);
}

void LSPStyle::onTextDeletedTotal(int position, int length, int linesAdded, const QString &text, int line)
{
}

void LSPStyle::onTextChangedTotal()
{
    if (d->textChangedTimer.isActive())
        d->textChangedTimer.stop();

    if (!d->editor)
        return;

    d->editor->SendScintilla(TextEditor::SCI_INDICATORCLEARRANGE, 0, d->editor->length());   // clean all indicator range style
    cleanDiagnostics();

    if (d->getClient()) {
        const auto &content = d->editor->text();
        qApp->metaObject()->invokeMethod(d->getClient(), "changeRequest",
                                         Q_ARG(const QString &, d->editor->getFile()),
                                         Q_ARG(const QByteArray &, content.toUtf8()));
        lsp::Position pos;
        d->editor->lineIndexFromPosition(d->textChangedCache.positionCache, &pos.line, &pos.character);
        pos.character += d->textChangedCache.lengthCache;
        qApp->metaObject()->invokeMethod(d->getClient(), "completionRequest",
                                         Q_ARG(const QString &, d->editor->getFile()),
                                         Q_ARG(const lsp::Position &, pos));
        qApp->metaObject()->invokeMethod(d->getClient(), "docSemanticTokensFull",
                                         Q_ARG(const QString &, d->editor->getFile()));
    }
}

void LSPStyle::onHovered(int position)
{
}

void LSPStyle::onHoverCleaned(int position)
{
}

void LSPStyle::onDefinitionHover(int position)
{
}

void LSPStyle::onDefinitionHoverCleaned(int position)
{
}

void LSPStyle::onIndicClicked(int position)
{
}

void LSPStyle::onIndicReleased(int position)
{
}

void LSPStyle::onSelectionMenu(QContextMenuEvent *event)
{
}

void LSPStyle::onContentReplaced(const QString &file, int start, int end, const QString &text)
{
}

void LSPStyle::onFileClosed(const QString &file)
{
}

void LSPStyle::renameRequest(const QString &newText)
{
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
