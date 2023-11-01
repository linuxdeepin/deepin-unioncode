// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stylelsp.h"
#include "Scintilla.h"
#include "stylesci.h"
#include "stylecolor.h"
#include "mainframe/texteditkeeper.h"
#include "textedittabwidget/textedit.h"
#include "textedittabwidget/textedittabwidget.h"
#include "textedittabwidget/style/lspclientkeeper.h"
#include "codelens/codelens.h"
#include "transceiver/codeeditorreceiver.h"
#include "renamepopup/renamepopup.h"
#include "base/abstractaction.h"

#include "common/common.h"
#include "framework/service/qtclassmanager.h"
#include "services/window/windowservice.h"

#include "Document.h"

#include <QHash>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QMenu>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QByteArray>
#include <QShortcut>

#include <bitset>

using namespace dpfservice;

class SciRangeCache
{
    std::optional<Scintilla::Position> start;
    std::optional<Scintilla::Position> end;
public:
    SciRangeCache(Scintilla::Position start, Scintilla::Position end)
        :start(start), end(end){}
    SciRangeCache(){}
    void clean() { start.reset(); end.reset(); }
    bool isEmpty() { return start && end;}
    Scintilla::Position getStart() const {return start.value();}
    void setStart(const Scintilla::Position &value) {start = value;}
    Scintilla::Position getEnd() const {return end.value();}
    void setEnd(const Scintilla::Position &value) {end = value;}
    bool operator == (const SciRangeCache &other){
        return start == other.start && end == other.end;
    }
};

class SciPositionCache
{
    Scintilla::Position sciPosition = -1;
public:
    SciPositionCache(){}
    SciPositionCache(const Scintilla::Position &pos)
        : sciPosition(pos){}
    void clean() {sciPosition = -1;}
    bool isEmpty() { return sciPosition == -1;}
    Scintilla::Position getSciPosition() const { return sciPosition;}
    void setSciPosition(const Scintilla::Position &value) { sciPosition = value;}
};

class DefinitionCache : public SciPositionCache
{
    std::optional<std::vector<newlsp::Location>> locations{};
    std::optional<std::vector<newlsp::LocationLink>> locationLinks{};
    std::optional<newlsp::Location>  location{};
    SciRangeCache textRange{};
    int cursor = 0; //Invalid
public:
    void clean() {
        cleanFromLsp();
        cursor = 0;
        SciPositionCache::clean();
        textRange.clean();
    }
    void cleanFromLsp() {
        if (locations) location.reset();
        if (location) location.reset();
        if (locationLinks) locationLinks.reset();
    }

    bool isEmpty() {
        return locations && location && locationLinks
                && cursor == 0 && SciPositionCache::isEmpty()
                && textRange.isEmpty();
    }
    std::vector<newlsp::Location> getLocations() const
    {
        if (locations.has_value())
            return locations.value();
        return {};
    }
    newlsp::Location getLocation() const {return location.value();}
    std::vector<newlsp::LocationLink> getLocationLinks() const {return locationLinks.value();}
    void set(const std::vector<newlsp::Location> &value) {locations = value;}
    void set(const newlsp::Location &value) {location = value;}
    void set(const std::vector<newlsp::LocationLink> &value) {locationLinks = value;}
    int getCursor() const {return cursor;}
    void setCursor(int value) {cursor = value;}
    SciRangeCache getTextRange() const {return textRange;}
    void setTextRange(const SciRangeCache &value) {textRange = value;}
};

class HoverCache : public SciPositionCache
{
public:
    void clean() { SciPositionCache::clean(); }
    bool isEmpty() { return SciPositionCache::isEmpty(); }
};

class RenameCache
{
    SciPositionCache start;
    SciPositionCache end;
public:
    void clean() { start.clean(); end.clean(); }
    bool isEmpty() { return start.isEmpty() && end.isEmpty(); }
    SciPositionCache getStart() const { return start; }
    void setStart(const SciPositionCache &value) {start = value;}
    SciPositionCache getEnd() const {return end;}
    void setEnd(const SciPositionCache &value) {end = value;}
};

class CompletionCache
{
public:
    lsp::CompletionProvider provider;
};

struct TextChangeCache
{
    enum State{
        Inserted,
        Deleted
    };
    State state;
    Scintilla::Position positionCache;
    Scintilla::Position lengthCache;
    Scintilla::Position linesAddedCache;
    QByteArray textCache;
    Scintilla::Position lineCache;
};

class StyleLspPrivate
{
    CompletionCache completionCache;
    DefinitionCache definitionCache;
    QTimer textChangedTimer;
    HoverCache hoverCache;
    RenamePopup renamePopup;
    RenameCache renameCache;
    TextEdit *edit{nullptr};
    TextChangeCache textChangedCache;
    QList<lsp::Data> tokensCache;
    static QAction *rangeFormattingAction;
    static QString formattingFile;
    friend class StyleLsp;
    newlsp::Client *getClient() const;
};

QAction *StyleLspPrivate::rangeFormattingAction = nullptr;
QString StyleLspPrivate::formattingFile = "";
// from ascii code
inline bool StyleLsp::isCharSymbol(const char ch) {
    return (ch >= 0x21 && ch < 0x2F + 1) || (ch >= 0x3A && ch < 0x40 + 1)
            || (ch >= 0x5B && ch < 0x60 + 1) || (ch >= 0x7B && ch < 0x7E + 1);
}

Sci_Position StyleLsp::getSciPosition(sptr_t doc, const newlsp::Position &pos)
{
    auto docTemp = (Scintilla::Internal::Document*)(doc);
    return docTemp->GetRelativePosition(docTemp->LineStart(pos.line), pos.character);
}

lsp::Position StyleLsp::getLspPosition(sptr_t doc, sptr_t sciPosition)
{
    auto docTemp = (Scintilla::Internal::Document*)(doc);
    int line = docTemp->LineFromPosition(sciPosition);
    Sci_Position lineChStartPos = getSciPosition(doc, newlsp::Position{line, 0});
    return lsp::Position{line, (int)(sciPosition - lineChStartPos)};
}

int editLineNumber(int originLineNumber)
{
    return originLineNumber + 1;
}

StyleLsp::StyleLsp(TextEdit *parent)
    : QObject (parent)
    , d (new StyleLspPrivate())
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowService>(WindowService::name());
    if (!windowService)
        return;
    d->edit = parent;

    setIndicStyle();
    setMargin();

    QObject::connect(d->edit, &ScintillaEditExtern::charAdded, this, &StyleLsp::SciCharAdded);
    QObject::connect(d->edit, &ScintillaEditExtern::linesAdded, this, &StyleLsp::sciLinesAdded);
    QObject::connect(d->edit, &ScintillaEditExtern::textInserted, this, &StyleLsp::sciTextInsertedTotal);
    QObject::connect(d->edit, &ScintillaEditExtern::textDeleted, this, &StyleLsp::sciTextDeletedTotal);
    QObject::connect(d->edit, &ScintillaEditExtern::hovered, this, &StyleLsp::sciHovered);
    QObject::connect(d->edit, &ScintillaEditExtern::hoverCleaned, this, &StyleLsp::sciHoverCleaned);
    QObject::connect(d->edit, &ScintillaEditExtern::definitionHover, this, &StyleLsp::sciDefinitionHover);
    QObject::connect(d->edit, &ScintillaEditExtern::definitionHoverCleaned, this, &StyleLsp::sciDefinitionHoverCleaned);
    QObject::connect(d->edit, &ScintillaEditExtern::indicClicked, this, &StyleLsp::sciIndicClicked);
    QObject::connect(d->edit, &ScintillaEditExtern::indicReleased, this, &StyleLsp::sciIndicReleased);
    QObject::connect(d->edit, &ScintillaEditExtern::selectionMenu, this, &StyleLsp::sciSelectionMenu);
    QObject::connect(d->edit, &ScintillaEditExtern::replaceed, this, &StyleLsp::sciReplaced);
    QObject::connect(d->edit, &ScintillaEditExtern::fileClosed, this, &StyleLsp::sciClosed);
    QObject::connect(&d->renamePopup, &RenamePopup::editingFinished, this, &StyleLsp::renameRequest, Qt::UniqueConnection);
    QObject::connect(CodeLens::instance(), &CodeLens::doubleClicked,
                     this, [=](const QString &filePath, const lsp::Range &range){
        newlsp::Range newRange;
        newRange.start.line = range.start.line;
        newRange.start.character = range.start.character;
        newRange.end.line = range.end.line;
        newRange.end.character = range.end.character;
        EditorCallProxy::instance()->toJumpFileLineWithKey(d->edit->projectKey(), filePath, editLineNumber(range.start.line));
    });

    if (!d->rangeFormattingAction) {
        d->rangeFormattingAction = new QAction(tr("Range Formatting"));
        ActionManager::getInstance()->registerAction(d->rangeFormattingAction, "Tool.Range.Formatting",
                                                     tr("Range Formatting"), QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_I),
                                                     ":/codeeditor/images/formatting.png");
        AbstractAction *actionImpl = new AbstractAction(d->rangeFormattingAction);
        windowService->addAction(MWM_TOOLS, actionImpl);
    }
}

TextEdit *StyleLsp::edit()
{
    return d->edit;
}

StyleLsp::~StyleLsp()
{

}

void StyleLsp::initLspConnection()
{
    if (!d->edit) {
        return;
    }

    QObject::connect(d->getClient(), QOverload<const lsp::References&>::of(&newlsp::Client::requestResult),
                     CodeLens::instance(), &CodeLens::displayReference, Qt::UniqueConnection);

    //bind signals to file diagnostics
    QObject::connect(d->getClient(), QOverload<const newlsp::PublishDiagnosticsParams &>::of(&newlsp::Client::publishDiagnostics),
                     this, [=](const newlsp::PublishDiagnosticsParams &data) {this->setDiagnostics(data);});

    QObject::connect(d->getClient(), QOverload<const QList<lsp::Data>&>::of(&newlsp::Client::requestResult),
                     this, &StyleLsp::setTokenFull);

    QObject::connect(d->getClient(), QOverload<const newlsp::Hover&>::of(&newlsp::Client::hoverRes),
                     this, &StyleLsp::setHover);

    QObject::connect(d->getClient(), QOverload<const lsp::CompletionProvider&>::of(&newlsp::Client::requestResult),
                     this, [=](const lsp::CompletionProvider& provider){
        d->completionCache.provider = provider;
    });

    QObject::connect(d->getClient(), &newlsp::Client::rangeFormattingRes,
                     this, &StyleLsp::rangeFormattingReplace);

    /* to use QOverload cast virtual slot can't working */
    QObject::connect(d->getClient(), QOverload<const newlsp::Location&>::of(&newlsp::Client::definitionRes),
                     this, QOverload<const newlsp::Location&>::of(&StyleLsp::setDefinition));
    QObject::connect(d->getClient(), QOverload<const std::vector<newlsp::Location>&>::of(&newlsp::Client::definitionRes),
                     this, QOverload<const std::vector<newlsp::Location> &>::of(&StyleLsp::setDefinition));
    QObject::connect(d->getClient(), QOverload<const std::vector<newlsp::LocationLink>&>::of(&newlsp::Client::definitionRes),
                     this, QOverload<const std::vector<newlsp::LocationLink>&>::of(&StyleLsp::setDefinition));

    if (d->getClient()) {
        qApp->metaObject()->invokeMethod(d->getClient(), "openRequest", Qt::QueuedConnection, Q_ARG(const QString &, d->edit->file()));
        qApp->metaObject()->invokeMethod(d->getClient(), "docSemanticTokensFull", Qt::QueuedConnection, Q_ARG(const QString &, d->edit->file()));
    }
}

int StyleLsp::getLspCharacter(sptr_t doc, sptr_t sciPosition)
{
    return getLspPosition(doc, sciPosition).character;
}

void StyleLsp::sciTextInsertedTotal(Scintilla::Position position,
                                    Scintilla::Position length, Scintilla::Position linesAdded,
                                    const QByteArray &text, Scintilla::Position line)
{
    if (!d->edit || !d->getClient())
        return;

    if (d->textChangedTimer.isActive()) {
        d->textChangedTimer.stop();
    }

    // 补全
    auto wordStartPos = d->edit->wordStartPosition(position - length, true);
    setCompletion(d->edit->textRange(wordStartPos, position).replace(" ","") + text,
                  position - wordStartPos, d->completionCache.provider);

    if (position == d->textChangedCache.positionCache + d->textChangedCache.lengthCache) {
        d->textChangedCache.lengthCache += length;
        d->textChangedCache.textCache += text;
    } else {
        d->textChangedCache.positionCache = position;
        d->textChangedCache.lengthCache = length;
        d->textChangedCache.textCache = text;
        d->textChangedCache.state = TextChangeCache::State::Inserted; //设置增量标志
    }

    d->textChangedTimer.start(500);
    QObject::connect(&d->textChangedTimer, &QTimer::timeout,
                     this, &StyleLsp::sciTextChangedTotal,
                     Qt::UniqueConnection);
}

void StyleLsp::sciTextDeletedTotal(Scintilla::Position position,
                                   Scintilla::Position length, Scintilla::Position linesAdded,
                                   const QByteArray &text, Scintilla::Position line)
{
    if (!d->edit || !d->getClient())
        return;

    if (d->textChangedTimer.isActive()) {
        d->textChangedTimer.stop();
    }

    if (d->textChangedCache.lengthCache == 0) { // 性质跳变
        d->textChangedCache.state = TextChangeCache::State::Deleted; // 设置为减量
        d->textChangedCache.positionCache = position + length;
        d->textChangedCache.lengthCache = length;
        d->textChangedCache.textCache.insert(0, text);
    }

    // 增量删除
    if (d->textChangedCache.state == TextChangeCache::State::Inserted) {
        if (d->textChangedCache.positionCache + d->textChangedCache.lengthCache - length == position && 0 != position) {
            d->textChangedCache.textCache.remove(d->textChangedCache.textCache.size() - length,
                                                 d->textChangedCache.textCache.size());
            d->textChangedCache.lengthCache -= length;
            d->textChangedCache.state = TextChangeCache::State::Inserted; // 设置增量标志
            //            // 补全
            //            auto wordStartPos = d->edit->wordStartPosition(position, true);
            //            auto wordEndPos = d->edit->wordEndPosition(position, true);
            //            qInfo() << "\npositionCache" << d->textChangedCache.positionCache
            //                    << "\ntextCache" << d->textChangedCache.textCache
            //                    << "\nlengthCache" << d->textChangedCache.lengthCache
            //                    << "\nstate" << d->textChangedCache.state
            //                    << "\nwodStartPos" << wordStartPos
            //                    << "\nwodEndPos" << wordEndPos
            //                    << "textRange" << d->edit->textRange(wordStartPos, wordEndPos);
            //            setCompletion(d->edit->textRange(wordStartPos, wordEndPos),
            //                          wordStartPos, d->completionCache.provider);
        }
    } else if (d->textChangedCache.state == TextChangeCache::State::Deleted){
        if (d->textChangedCache.positionCache == position + d->textChangedCache.lengthCache) {
            d->textChangedCache.lengthCache += length; // 删除文字数量统计
            d->textChangedCache.textCache.insert(0, text);  // 删除文字总览
        }
    }

    d->textChangedTimer.start(500);
    QObject::connect(&d->textChangedTimer, &QTimer::timeout,
                     this, &StyleLsp::sciTextChangedTotal,
                     Qt::UniqueConnection);
}

void StyleLsp::sciLinesAdded(Scintilla::Position position)
{
    Q_UNUSED(position)
    //notiong to do
}

void StyleLsp::SciCharAdded(int ch)
{
    if (ch == '\n') {
        auto currLine = d->edit->lineFromPosition(d->edit->currentPos());
        if (currLine > 0){
            auto upLineIndentation = d->edit->lineIndentation(currLine - 1);
            int count = 0;
            std::string indent;
            while (count < upLineIndentation) {
                indent += " ";
                count ++;
            }
            d->edit->addText(indent.size(), indent.c_str());
        }
    }
}

void StyleLsp::sciTextChangedTotal()
{
    if (d->textChangedTimer.isActive())
        d->textChangedTimer.stop();

    if (!d->edit)
        return;

    d->edit->indicatorClearRange(0, d->edit->length()); // clean all indicator range style
    cleanDiagnostics();

    if (d->getClient()) {
        qApp->metaObject()->invokeMethod(d->getClient(), "changeRequest",
                                         Q_ARG(const QString &, d->edit->file()),
                                         Q_ARG(const QByteArray &, d->edit->textRange(0, d->edit->length())));
        qApp->metaObject()->invokeMethod(d->getClient(), "completionRequest",
                                         Q_ARG(const QString &, d->edit->file()),
                                         Q_ARG(const lsp::Position &, getLspPosition(d->edit->docPointer(), d->textChangedCache.positionCache)));
        qApp->metaObject()->invokeMethod(d->getClient(), "docSemanticTokensFull",
                                         Q_ARG(const QString &, d->edit->file()));
    }
};

void StyleLsp::sciHovered(Scintilla::Position position)
{
    if (!d->edit)
        return;

    if (d->edit->isLeave())
        return;

    d->hoverCache.setSciPosition(position);

    if(d->getClient()) {
        qApp->metaObject()->invokeMethod(d->getClient(), "docHoverRequest",
                                         Q_ARG(const QString &, d->edit->file()),
                                         Q_ARG(const lsp::Position &, getLspPosition(d->edit->docPointer(), d->hoverCache.getSciPosition())));
    }
}

void StyleLsp::sciHoverCleaned(Scintilla::Position position)
{
    Q_UNUSED(position);
    if (!d->edit)
        return;

    cleanHover();
    d->hoverCache.clean();
}

void StyleLsp::sciDefinitionHover(Scintilla::Position position)
{
    if (!d->edit || d->edit->isLeave())
        return;

    // 判断缓存文字范围
    auto afterTextRange = d->definitionCache.getTextRange();
    auto currTextRange = SciRangeCache{d->edit->wordStartPosition(position, true), d->edit->wordEndPosition(position, true)};
    auto isSameTextRange = afterTextRange == currTextRange;

    if (isSameTextRange) { // 相同的关键字不再触发Definition的绘制
        d->definitionCache.setSciPosition(position); // 更新坐标点
        return;
    } else {
        d->definitionCache.setTextRange(currTextRange);
        d->definitionCache.setSciPosition(position);
        d->definitionCache.cleanFromLsp();
    }

    auto lspPostion = getLspPosition(d->edit->docPointer(), d->definitionCache.getSciPosition());
    if (d->getClient()){
        qApp->metaObject()->invokeMethod(d->getClient(), "definitionRequest",
                                         Q_ARG(const QString &, d->edit->file()),
                                         Q_ARG(const lsp::Position &, lspPostion));
    }
}

void StyleLsp::sciDefinitionHoverCleaned(Scintilla::Position position)
{
    Q_UNUSED(position);
    if (!d->edit)
        return;

    // 判断缓存文字范围
    auto afterTextRange = d->definitionCache.getTextRange();
    auto currTextRange = SciRangeCache{d->edit->wordStartPosition(position, true), d->edit->wordEndPosition(position, true)};
    auto isSameTextRange = afterTextRange == currTextRange;
    if (!d->definitionCache.isEmpty() && !isSameTextRange) {
        if (d->edit) {
            cleanDefinition(d->definitionCache.getSciPosition());
        }
        d->definitionCache.clean();
    }
}

void StyleLsp::sciIndicClicked(Scintilla::Position position)
{
    Q_UNUSED(position);
    if (!d->edit)
        return;

    std::bitset<32> flags(d->edit->indicatorAllOnFor(position));
    if (flags[INDIC_COMPOSITIONTHICK]) {
        if (d->definitionCache.getLocations().size() > 0) {
            auto one = d->definitionCache.getLocations().front();
            EditorCallProxy::instance()->toJumpFileLineWithKey(d->edit->projectKey(),
                                                               QUrl(QString::fromStdString(one.uri)).toLocalFile(),
                                                               editLineNumber(one.range.end.line));
            cleanDefinition(position);
        } else if (d->definitionCache.getLocationLinks().size() > 0) {
            auto one = d->definitionCache.getLocationLinks().front();
            EditorCallProxy::instance()->toJumpFileLineWithKey(d->edit->projectKey(),
                                                               QUrl(QString::fromStdString(one.targetUri)).toLocalFile(),
                                                               editLineNumber(one.targetRange.end.line));
            cleanDefinition(position);
        } else {
            auto one = d->definitionCache.getLocation();
            EditorCallProxy::instance()->toJumpFileLineWithKey(d->edit->projectKey(),
                                                               QUrl(QString::fromStdString(one.uri)).toLocalFile(),
                                                               editLineNumber(one.range.end.line));
            cleanDefinition(position);
        }
    }
}

void StyleLsp::sciIndicReleased(Scintilla::Position position)
{
    Q_UNUSED(position);
}

void StyleLsp::sciSelectionMenu(QContextMenuEvent *event)
{
    if (!d->edit)
        return;

    d->renameCache.setStart(d->edit->selectionStart());
    d->renameCache.setEnd(d->edit->selectionEnd());

    QPoint showPos = d->edit->mapToGlobal(event->pos());
    QByteArray sourceText = d->edit->textRange(
                d->edit->wordStartPosition(d->edit->selectionStart(), true),
                d->edit->wordEndPosition(d->edit->selectionEnd(), true));

    QMenu contextMenu;
    QMenu refactor(QMenu::tr("Refactor"));

    QAction *renameAction = refactor.addAction(QAction::tr("Rename"));
    QObject::connect(renameAction, &QAction::triggered, [&](){
        d->renamePopup.setOldName(sourceText);
        d->renamePopup.exec(showPos);
    });
    contextMenu.addMenu(&refactor);

    QAction *findUsageAction = contextMenu.addAction(QAction::tr("Find Usages"));
    QObject::connect(findUsageAction, &QAction::triggered, [&](){
        if (d->getClient()) {
            auto lspPos = getLspPosition(d->edit->docPointer(), d->edit->selectionStart());
            qApp->metaObject()->invokeMethod(d->getClient(), "referencesRequest",
                                             Q_ARG(const QString &, d->edit->file()),
                                             Q_ARG(const lsp::Position &, lspPos));
        }
    });

    contextMenu.addAction(d->rangeFormattingAction);
    QObject::connect(d->rangeFormattingAction, &QAction::triggered, [&](){
        if (d->getClient()) {
            auto selStart = getLspPosition(d->edit->docPointer(), d->edit->selectionStart());
            auto selEnd = getLspPosition(d->edit->docPointer(), d->edit->selectionEnd());
            newlsp::Position newSelStart = {selStart.line, selStart.character};
            newlsp::Position newSelEnd = {selEnd.line, selEnd.character};
            newlsp::DocumentRangeFormattingParams params;
            params.textDocument.uri = QUrl::fromLocalFile(d->edit->file())
                    .toString().toStdString();
            params.range = newlsp::Range{newSelStart, newSelEnd};
            params.options.tabSize = 4;
            params.options.insertSpaces = true;
            d->formattingFile = d->edit->file();
            qApp->metaObject()->invokeMethod(d->getClient(), "rangeFormatting",
                                             Q_ARG(const newlsp::DocumentRangeFormattingParams &, params));
        }
    });

    // notify other plugin to add action.
    editor.contextMenu(QVariant::fromValue(&contextMenu));

    contextMenu.move(showPos);
    contextMenu.exec();
}

void StyleLsp::sciReplaced(const QString &file, Scintilla::Position start, Scintilla::Position end, const QString &text)
{
    if (!d->edit || file != d->formattingFile)
        return;
    Q_UNUSED(text)
    Q_UNUSED(start);
    Q_UNUSED(end);
    if (d->getClient()) {
        qApp->metaObject()->invokeMethod(d->getClient(), "changeRequest",
                                         Q_ARG(const QString &, file),
                                         Q_ARG(const QByteArray &, d->edit->textRange(0, d->edit->length())));
    }
}

void StyleLsp::sciClosed(const QString &file)
{
    if (d->getClient()) {
        qApp->metaObject()->invokeMethod(d->getClient(), "closeRequest", Q_ARG(const QString &, file));
    }
}

void StyleLsp::renameRequest(const QString &newText)
{
    auto sciPostion = d->renameCache.getStart().getSciPosition();
    if (d->edit) {
        if (d->getClient()) {
            qApp->metaObject()->invokeMethod(d->getClient(), "renameRequest",
                                             Q_ARG(const QString &, d->edit->file()),
                                             Q_ARG(const lsp::Position &, getLspPosition(d->edit->docPointer(), sciPostion)),
                                             Q_ARG(const QString &, newText));
        }
    }
}

StyleLsp::IndicStyleExt StyleLsp::symbolIndic(lsp::SemanticTokenType::type_value token,
                                              QList<lsp::SemanticTokenType::type_index> modifier)
{
    Q_UNUSED(d->edit);
    Q_UNUSED(token);
    Q_UNUSED(modifier);
    return {};
}

lsp::SemanticTokenType::type_value StyleLsp::tokenToDefine(int token)
{
    auto client = d->getClient();
    if (!client)
        return {};
    auto initSecTokensProvider = client->initSecTokensProvider();
    if (0 <= token && token < initSecTokensProvider.legend.tokenTypes.size())
        return initSecTokensProvider.legend.tokenTypes[token];
    return {};
}

void StyleLsp::setIndicStyle()
{
    d->edit->indicSetStyle(INDIC_PLAIN, INDIC_PLAIN);
    d->edit->indicSetStyle(INDIC_SQUIGGLE, INDIC_SQUIGGLE);
    d->edit->indicSetStyle(INDIC_TT, INDIC_TT);
    d->edit->indicSetStyle(INDIC_DIAGONAL, INDIC_DIAGONAL);
    d->edit->indicSetStyle(INDIC_STRIKE, INDIC_STRIKE);
    d->edit->indicSetStyle(INDIC_HIDDEN, INDIC_HIDDEN);
    d->edit->indicSetStyle(INDIC_BOX, INDIC_BOX);
    d->edit->indicSetStyle(INDIC_ROUNDBOX, INDIC_ROUNDBOX);
    d->edit->indicSetStyle(INDIC_STRAIGHTBOX, INDIC_STRAIGHTBOX);
    d->edit->indicSetStyle(INDIC_FULLBOX, INDIC_FULLBOX);
    d->edit->indicSetStyle(INDIC_DASH, INDIC_DASH);
    d->edit->indicSetStyle(INDIC_DOTS, INDIC_DOTS);
    d->edit->indicSetStyle(INDIC_SQUIGGLELOW, INDIC_SQUIGGLELOW);
    d->edit->indicSetStyle(INDIC_DOTBOX, INDIC_DOTBOX);
    d->edit->indicSetStyle(INDIC_GRADIENT, INDIC_GRADIENT);
    d->edit->indicSetStyle(INDIC_GRADIENTCENTRE, INDIC_GRADIENTCENTRE);
    d->edit->indicSetStyle(INDIC_SQUIGGLEPIXMAP, INDIC_SQUIGGLEPIXMAP);
    d->edit->indicSetStyle(INDIC_COMPOSITIONTHICK, INDIC_COMPOSITIONTHICK);
    d->edit->indicSetStyle(INDIC_COMPOSITIONTHIN, INDIC_COMPOSITIONTHIN);
    d->edit->indicSetStyle(INDIC_TEXTFORE, INDIC_TEXTFORE);
    d->edit->indicSetStyle(INDIC_POINT, INDIC_POINT);
    d->edit->indicSetStyle(INDIC_POINTCHARACTER, INDIC_POINTCHARACTER);
}

void StyleLsp::setMargin()
{
    d->edit->setMargins(SC_MAX_MARGIN);
    d->edit->setMarginTypeN(Margin::LspCustom, SC_MARGIN_SYMBOL);
    d->edit->setMarginWidthN(Margin::LspCustom, 16);
    d->edit->setMarginMaskN(Margin::LspCustom, 1 << MarkerNumber::Error | 1 << MarkerNumber::ErrorLineBackground
                            | 1 << MarkerNumber::Warning | 1 << MarkerNumber::WarningLineBackground
                            | 1 << MarkerNumber::Information | 1 << MarkerNumber::InformationLineBackground
                            | 1 << MarkerNumber::Hint | 1 << MarkerNumber::HintLineBackground);

    d->edit->markerDefine(MarkerNumber::Error, SC_MARK_CIRCLE);
    d->edit->markerDefine(MarkerNumber::Warning, SC_MARK_CIRCLE);
    d->edit->markerDefine(MarkerNumber::Information, SC_MARK_CIRCLE);
    d->edit->markerDefine(MarkerNumber::Hint, SC_MARK_CIRCLE);

    d->edit->markerDefine(MarkerNumber::ErrorLineBackground, SC_MARK_BACKGROUND);
    d->edit->markerDefine(MarkerNumber::WarningLineBackground, SC_MARK_BACKGROUND);
    d->edit->markerDefine(MarkerNumber::InformationLineBackground, SC_MARK_BACKGROUND);
    d->edit->markerDefine(MarkerNumber::HintLineBackground, SC_MARK_BACKGROUND);

    d->edit->markerSetFore(MarkerNumber::Error, StyleColor::color(StyleColor::Table::get()->Red));
    d->edit->markerSetBackTranslucent(MarkerNumber::Error, 0);
    d->edit->markerSetStrokeWidth(MarkerNumber::Error, 300);

    d->edit->markerSetFore(MarkerNumber::Warning, StyleColor::color(StyleColor::Table::get()->Yellow));
    d->edit->markerSetBackTranslucent(MarkerNumber::Warning, 0);
    d->edit->markerSetStrokeWidth(MarkerNumber::Warning, 300);

    d->edit->markerSetFore(MarkerNumber::Information, StyleColor::color(StyleColor::Table::get()->Yellow));
    d->edit->markerSetBackTranslucent(MarkerNumber::Information, 0);
    d->edit->markerSetStrokeWidth(MarkerNumber::Information, 300);

    d->edit->markerSetFore(MarkerNumber::Hint, StyleColor::color(StyleColor::Table::get()->Yellow));
    d->edit->markerSetBackTranslucent(MarkerNumber::Hint, 0);
    d->edit->markerSetStrokeWidth(MarkerNumber::Hint, 300);

    d->edit->markerSetFore(MarkerNumber::ErrorLineBackground, StyleColor::color(StyleColor::Table::get()->Red));
    d->edit->markerSetBack(MarkerNumber::ErrorLineBackground, StyleColor::color(StyleColor::Table::get()->Red));
    d->edit->markerSetAlpha(MarkerNumber::ErrorLineBackground, 0x22);
    d->edit->markerSetFore(MarkerNumber::WarningLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    d->edit->markerSetBack(MarkerNumber::WarningLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    d->edit->markerSetAlpha(MarkerNumber::WarningLineBackground, 0x22);
    d->edit->markerSetFore(MarkerNumber::InformationLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    d->edit->markerSetBack(MarkerNumber::InformationLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    d->edit->markerSetAlpha(MarkerNumber::InformationLineBackground, 0x22);
    d->edit->markerSetFore(MarkerNumber::HintLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    d->edit->markerSetBack(MarkerNumber::HintLineBackground, StyleColor::color(StyleColor::Table::get()->Yellow));
    d->edit->markerSetAlpha(MarkerNumber::HintLineBackground, 0x22);
}

void StyleLsp::setDiagnostics(const newlsp::PublishDiagnosticsParams &data)
{
    if (!edit())
        return;

    if (QUrl(QString::fromStdString(data.uri)).toLocalFile() != edit()->file())
        return;

    this->cleanDiagnostics();
    for (auto val : data.diagnostics) {
        if (newlsp::Enum::DiagnosticSeverity::get()->Error == val.severity.value()) { // error
            newlsp::Position start{val.range.start.line, val.range.start.character};
            newlsp::Position end{val.range.end.line, val.range.start.character};
            Sci_Position startPos = getSciPosition(d->edit->docPointer(), start);
            Sci_Position endPos = getSciPosition(d->edit->docPointer(), end);
            d->edit->setIndicatorCurrent(INDIC_SQUIGGLE);
            d->edit->indicSetFore(INDIC_SQUIGGLE, StyleColor::color(StyleColor::Table::get()->Red));
            d->edit->indicatorFillRange(startPos, endPos - startPos);
            std::string message = "Error: " + val.message.value();
            d->edit->eOLAnnotationSetText(val.range.start.line, message.c_str());
            d->edit->eOLAnnotationSetStyleOffset(EOLAnnotation::RedTextFore);
            d->edit->eOLAnnotationSetStyle(val.range.start.line, EOLAnnotation::RedTextFore - d->edit->eOLAnnotationStyleOffset());
            d->edit->styleSetFore(EOLAnnotation::RedTextFore, StyleColor::color(StyleColor::Table::get()->Red));
            d->edit->eOLAnnotationSetVisible(EOLANNOTATION_STANDARD);
            d->edit->markerAdd(val.range.start.line, Error);
            d->edit->markerAdd(val.range.start.line, ErrorLineBackground);
        }
    }
}

void StyleLsp::cleanDiagnostics()
{
    d->edit->eOLAnnotationClearAll();
    for (int line = 0; line < d->edit->lineCount(); line ++) {
        d->edit->markerDelete(line, Error);
        d->edit->markerDelete(line, ErrorLineBackground);
        d->edit->markerDelete(line, Warning);
        d->edit->markerDelete(line, WarningLineBackground);
        d->edit->markerDelete(line, Information);
        d->edit->markerDelete(line, InformationLineBackground);
        d->edit->markerDelete(line, Hint);
        d->edit->markerDelete(line, HintLineBackground);
    }
}

void StyleLsp::setTokenFull(const QList<lsp::Data> &tokens)
{
    qInfo() << Q_FUNC_INFO << tokens.size();
    if (!edit())
        return;

    if (!d->edit->lexer())
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
        auto sciStartPos = StyleLsp::getSciPosition(d->edit->docPointer(), {cacheLine, val.start.character});
        auto sciEndPos = d->edit->wordEndPosition(sciStartPos, true);
        auto wordStartPos = d->edit->wordStartPosition(sciStartPos, true);
        if (sciStartPos == 0 || sciEndPos == d->edit->length() || wordStartPos != sciStartPos)
            continue;
        QString sourceText = d->edit->textRange(sciStartPos, sciEndPos);
        int wordLength = sciEndPos - sciStartPos;
#ifdef QT_DEBUG
        qInfo() << "text:" << sourceText;
#endif
        if (!sourceText.isEmpty() && wordLength == val.length) {
            QString tokenValue = tokenToDefine(val.tokenType);
#ifdef QT_DEBUG
            qInfo() << "tokenValue:" << tokenValue;
#endif
            auto indics = symbolIndic(tokenValue, val.tokenModifiers);
            for (int i = 0; i < INDIC_MAX; i++) {
                if (indics.fore.keys().contains(i)) {
#ifdef QT_DEBUG
                    qInfo() << "fillRangeColor:" << hex << indics.fore[i];
#endif
                    d->edit->setIndicatorCurrent(i);
                    d->edit->indicSetFlags(i, SC_INDICFLAG_VALUEFORE);
                    d->edit->setIndicatorValue(indics.fore[i]);
                    d->edit->indicatorFillRange(sciStartPos, wordLength);
                }
            }

            QString tokenAnnLine = TextEditKeeper::getTokenTypeAnnLine(tokenValue, sourceText);
            if (!tokenAnnLine.isEmpty()) {
                editor.setAnnotation(d->edit->file(), cacheLine,
                                     QString(TextEditKeeper::userActionAnalyseTitle()),
                                     AnnotationInfo{tokenAnnLine});
            }
        }
    }
}

void StyleLsp::cleanTokenFull()
{
    Q_UNUSED(d->edit);
}

void StyleLsp::setHover(const newlsp::Hover &hover)
{
    if (!edit() || edit()->isLeave())
        return;

    d->edit->callTipSetBack(STYLE_DEFAULT);

    std::string showText;
    if (newlsp::any_contrast<std::vector<newlsp::MarkedString>>(hover.contents)) {
        auto markupStrings = std::any_cast<std::vector<newlsp::MarkedString>>(hover.contents);
        for (auto one : markupStrings) {
            if (!showText.empty()) showText += "\n";

            if (!one.value.empty()) // markedString value append
                showText += one.value;
            else if (!std::string(one).empty()) // markedString self is String append
                showText += one;
        };
    } else if (newlsp::any_contrast<newlsp::MarkupContent>(hover.contents)){
        auto markupContent = std::any_cast<newlsp::MarkupContent>(hover.contents);
        showText = markupContent.value;
    } else if (newlsp::any_contrast<newlsp::MarkedString>(hover.contents)){
        auto markedString = std::any_cast<newlsp::MarkedString>(hover.contents);
        if (!std::string(markedString).empty()) {
            showText = std::string(markedString);
        } else {
            showText = markedString.value;
        }
    }

    if (hover.range) {
        // noting to do
    }

    if (!showText.empty())
        d->edit->callTipShow(d->hoverCache.getSciPosition(), showText.c_str());

    d->hoverCache.clean();
}

void StyleLsp::cleanHover()
{
    d->edit->callTipCancel();
}

void StyleLsp::setDefinition(const newlsp::Location &data)
{
    if (!edit() || edit()->isLeave())
        return;

    d->definitionCache.set(data);
    auto sciStartPos = d->edit->wordStartPosition(d->definitionCache.getSciPosition(), true);
    auto sciEndPos = d->edit->wordEndPosition(d->definitionCache.getSciPosition(), true);

    setDefinitionSelectedStyle(sciStartPos, sciEndPos);
}

void StyleLsp::setDefinition(const std::vector<newlsp::Location> &data)
{
    if (!edit() || edit()->isLeave())
        return;

    d->definitionCache.set(data);
    auto sciStartPos = d->edit->wordStartPosition(d->definitionCache.getSciPosition(), true);
    auto sciEndPos = d->edit->wordEndPosition(d->definitionCache.getSciPosition(), true);

    if (data.size() >= 1) {
        setDefinitionSelectedStyle(sciStartPos, sciEndPos);
    }
}

void StyleLsp::setDefinition(const std::vector<newlsp::LocationLink> &data)
{
    if (!edit() || edit()->isLeave())
        return;

    d->definitionCache.set(data);
    auto sciStartPos = d->edit->wordStartPosition(d->definitionCache.getSciPosition(), true);
    auto sciEndPos = d->edit->wordEndPosition(d->definitionCache.getSciPosition(), true);

    if (data.size() >= 1) {
        setDefinitionSelectedStyle(sciStartPos, sciEndPos);
    }
}

void StyleLsp::cleanDefinition(const Scintilla::Position &pos)
{
    std::bitset<32> flags(d->edit->indicatorAllOnFor(pos));
    if (flags[INDIC_COMPOSITIONTHICK]) {
        // auto hotSpotStart = d->edit->indicatorStart(INDIC_COMPOSITIONTHICK, pos);
        // auto hotSpotEnd = d->edit->indicatorEnd(INDIC_COMPOSITIONTHICK, pos);
        d->edit->setCursor(d->definitionCache.getCursor());
        // d->edit->indicatorClearRange(hotSpotStart, hotSpotEnd);
        d->edit->indicatorClearRange(0, d->edit->length());
    }
}

void StyleLsp::rangeFormattingReplace(const std::vector<newlsp::TextEdit> &edits)
{
    if (edits.size() <= 0)
        return;

    for (auto itera = edits.rbegin(); itera != edits.rend(); itera++) {
        auto sciPosStart = getSciPosition(d->edit->docPointer(), itera->range.start);
        auto sciPosEnd = getSciPosition(d->edit->docPointer(), itera->range.end);
        auto newText = QString::fromStdString(itera->newText);
        QString curFile = d->edit->file();
        QString curFormatting = d->formattingFile;
        if (d->edit->file() == d->formattingFile)
            d->edit->replaceRange(sciPosStart, sciPosEnd, newText);
    }
}

void StyleLsp::setDefinitionSelectedStyle(const Scintilla::Position start, const Scintilla::Position end)
{
    d->edit->setIndicatorCurrent(INDIC_COMPOSITIONTHICK);
    d->edit->indicSetFore(INDIC_COMPOSITIONTHICK, d->edit->styleFore(0));
    d->edit->indicatorFillRange(start, end - start);
    if (d->edit->cursor() != 8) {
        d->definitionCache.setCursor(d->edit->cursor());
        d->edit->setCursor(8); // hand from Scintilla platfrom.h
    }
}

void StyleLsp::setCompletion(const QByteArray &text,
                             const Scintilla::Position enterLenght,
                             const lsp::CompletionProvider &provider)
{
    if (d->edit->isLeave())
        return;

    if (provider.items.isEmpty() || d->textChangedCache.textCache.isEmpty())
        return;

    const unsigned char sep = 0x7C; // "|"
    d->edit->autoCSetSeparator((sptr_t)sep);
    QString inserts;
    for (auto item : provider.items) {
        if (!item.insertText.startsWith(text))
            continue;
        inserts += (item.insertText += sep);
    }
    if (inserts.endsWith(sep)){
        inserts.remove(inserts.count() - 1 , 1);
    }

    if (inserts.isEmpty())
        return;

    d->edit->autoCShow(enterLenght, inserts.toUtf8());
}

newlsp::Client *StyleLspPrivate::getClient() const
{
    if (edit) {
        return LSPClientKeeper::instance()->get(edit->projectKey());
    }
    return nullptr;
}
