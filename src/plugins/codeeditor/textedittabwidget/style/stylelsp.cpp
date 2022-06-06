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
#include "stylelsp.h"
#include "Scintilla.h"
#include "stylesci.h"
#include "stylecolor.h"
#include "textedittabwidget/textedit.h"
#include "textedittabwidget/textedittabwidget.h"
#include "refactorwidget/refactorwidget.h"
#include "renamepopup/renamepopup.h"

#include "common/common.h"
#include "framework/service/qtclassmanager.h"

#include "Document.h"

#include <QHash>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QMenu>
#include <QLineEdit>
#include <QVBoxLayout>

#include <bitset>

class SciRangeCache
{
    Scintilla::Position start = -1;
    Scintilla::Position end = -1;
public:
    SciRangeCache(Scintilla::Position start, Scintilla::Position end)
        :start(start), end(end){}
    SciRangeCache(){}
    void clean()
    {
        start = -1;
        end = -1;
    }
    bool isEmpty()
    {
        return start != -1 && end != -1;
    }
    Scintilla::Position getStart() const
    {
        return start;
    }
    void setStart(const Scintilla::Position &value)
    {
        start = value;
    }
    Scintilla::Position getEnd() const
    {
        return end;
    }
    void setEnd(const Scintilla::Position &value)
    {
        end = value;
    }
    bool operator == (const SciRangeCache &other)
    {
        return start == other.start
                && end == other.end;
    }
};

class SciPositionCache
{
    Scintilla::Position sciPosition = -1;
public:
    SciPositionCache(){}
    SciPositionCache(const Scintilla::Position &pos)
        : sciPosition(pos){}
    void clean()
    {
        sciPosition = -1;
    }
    bool isEmpty()
    {
        return sciPosition == -1;
    }
    Scintilla::Position getSciPosition() const
    {
        return sciPosition;
    }
    void setSciPosition(const Scintilla::Position &value)
    {
        sciPosition = value;
    }
};

class DefinitionCache : public SciPositionCache
{
    lsp::DefinitionProvider provider{};
    SciRangeCache textRange{};
    int cursor = 0; //Invalid
public:
    void clean()
    {
        provider.clear();
        cursor = 0;
        SciPositionCache::clean();
        textRange.clean();
    }
    bool isEmpty()
    {
        return provider.isEmpty()
                && cursor == 0
                && SciPositionCache::isEmpty()
                && textRange.isEmpty();
    }
    lsp::DefinitionProvider getProvider() const
    {
        return provider;
    }
    void setProvider(const lsp::DefinitionProvider &value)
    {
        provider = value;
    }
    int getCursor() const
    {
        return cursor;
    }
    void setCursor(int value)
    {
        cursor = value;
    }
    SciRangeCache getTextRange() const
    {
        return textRange;
    }
    void setTextRange(const SciRangeCache &value)
    {
        textRange = value;
    }
};

class HoverCache : public SciPositionCache
{
public:
    void clean()
    {
        SciPositionCache::clean();
    }
    bool isEmpty()
    {
        return SciPositionCache::isEmpty();
    }
};

class RenameCache
{
    SciPositionCache start;
    SciPositionCache end;
public:
    void clean()
    {
        start.clean();
        end.clean();
    }
    bool isEmpty()
    {
        return start.isEmpty()
                && end.isEmpty();
    }
    SciPositionCache getStart() const
    {
        return start;
    }
    void setStart(const SciPositionCache &value)
    {
        start = value;
    }
    SciPositionCache getEnd() const
    {
        return end;
    }
    void setEnd(const SciPositionCache &value)
    {
        end = value;
    }
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
    lsp::Client *lspClient;
    friend class StyleLsp;
};

// from ascii code
inline bool StyleLsp::isCharSymbol(const char ch) {
    return (ch >= 0x21 && ch < 0x2F + 1) || (ch >= 0x3A && ch < 0x40 + 1)
            || (ch >= 0x5B && ch < 0x60 + 1) || (ch >= 0x7B && ch < 0x7E + 1);
}

Sci_Position StyleLsp::getSciPosition(sptr_t doc, const lsp::Position &pos)
{
    auto docTemp = (Scintilla::Internal::Document*)(doc);
    return docTemp->GetRelativePosition(docTemp->LineStart(pos.line), pos.character);
}

lsp::Position StyleLsp::getLspPosition(sptr_t doc, sptr_t sciPosition)
{
    auto docTemp = (Scintilla::Internal::Document*)(doc);
    int line = docTemp->LineFromPosition(sciPosition);
    Sci_Position lineChStartPos = getSciPosition(doc, lsp::Position{line, 0});
    return lsp::Position{line, (int)(sciPosition - lineChStartPos)};
}

StyleLsp::StyleLsp(TextEdit *parent)
    : QObject (parent)
    , d (new StyleLspPrivate())
{
    d->edit = parent;

    setIndicStyle();
    setMargin();

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
    QObject::connect(&d->renamePopup, &RenamePopup::editingFinished, this, &StyleLsp::renameRequest, Qt::UniqueConnection);
    QObject::connect(RefactorWidget::instance(), &RefactorWidget::doubleClicked,
                     this, [=](const QString &filePath, const lsp::Range &range){
        TextEditTabWidget::instance()->jumpToRange(filePath, range);
    });
}

TextEdit *StyleLsp::edit()
{
    return d->edit;
}

StyleLsp::~StyleLsp()
{

}

void StyleLsp::setClient(lsp::Client *client)
{
    d->lspClient = client;
}

lsp::Client *StyleLsp::getClient() const
{
    return d->lspClient;
}

void StyleLsp::initLspConnection()
{
    if (!d->edit) {
        return;
    }

    QObject::connect(getClient(), QOverload<const lsp::References&>::of(&lsp::Client::requestResult),
                     RefactorWidget::instance(), &RefactorWidget::displayReference, Qt::UniqueConnection);

    //bind signals to file diagnostics
    QObject::connect(getClient(), QOverload<const lsp::DiagnosticsParams &>::of(&lsp::Client::notification),
                     this, &StyleLsp::setDiagnostics);

    QObject::connect(getClient(), QOverload<const QList<lsp::Data>&>::of(&lsp::Client::requestResult),
                     this, &StyleLsp::setTokenFull);

    QObject::connect(getClient(), QOverload<const lsp::Hover&>::of(&lsp::Client::requestResult),
                     this, &StyleLsp::setHover);

    QObject::connect(getClient(), QOverload<const lsp::CompletionProvider&>::of(&lsp::Client::requestResult),
                     this, [=](const lsp::CompletionProvider& provider){
        d->completionCache.provider = provider;
    });

    QObject::connect(getClient(), QOverload<const lsp::DefinitionProvider&>::of(&lsp::Client::requestResult),
                     this, &StyleLsp::setDefinition);

    if (getClient()) {
        getClient()->openRequest(d->edit->file());
        getClient()->docSemanticTokensFull(d->edit->file());
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
    if (!d->edit || !getClient())
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
    if (!d->edit || !getClient())
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

void StyleLsp::sciTextChangedTotal()
{
    if (d->textChangedTimer.isActive())
        d->textChangedTimer.stop();

    if (!d->edit)
        return;

    d->edit->indicatorClearRange(0, d->edit->length()); // clean all indicator range style
    cleanDiagnostics();

    if (getClient()) {
        getClient()->changeRequest(d->edit->file(), d->edit->textRange(0, d->edit->length()));
        getClient()->completionRequest(d->edit->file(), getLspPosition(d->edit->docPointer(), d->textChangedCache.positionCache));
        getClient()->docSemanticTokensFull(d->edit->file());
    }
};

void StyleLsp::sciHovered(Scintilla::Position position)
{
    if (!d->edit)
        return;

    if (d->edit->isLeave())
        return;

    d->hoverCache.setSciPosition(position);

    auto lspPostion = getLspPosition(d->edit->docPointer(), d->hoverCache.getSciPosition());

    if(getClient()) {
        getClient()->docHoverRequest(d->edit->file(), lspPostion);
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

    // 编辑器不相等, 直接刷新数据
    if  (d->edit != d->edit) {
        d->definitionCache.setSciPosition(position);
        d->definitionCache.setTextRange(currTextRange);
        d->definitionCache.setProvider({}); // 清空Provider
        d->edit->setCursor(-1); // 恢复鼠标状态
        d->definitionCache.setCursor(d->edit->cursor());
    } else { // 编辑器相等
        if (isSameTextRange) { // 相同的关键字不再触发Definition的绘制
            d->definitionCache.setSciPosition(position); // 更新坐标点
            return;
        } else {
            d->definitionCache.setTextRange(currTextRange);
            d->definitionCache.setSciPosition(position);
            d->definitionCache.setProvider({}); // 清空Provider
        }
    }
    auto lspPostion = getLspPosition(d->edit->docPointer(), d->definitionCache.getSciPosition());
    if (getClient()){
        getClient()->definitionRequest(d->edit->file(), lspPostion);
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
        if (d->definitionCache.getProvider().count() > 0) {
            auto providerAtOne = d->definitionCache.getProvider().first();
            TextEditTabWidget::instance()->jumpToLine(d->edit->projectHead(),
                                                      providerAtOne.fileUrl.toLocalFile(),
                                                      providerAtOne.range.end.line);
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

    QAction *findSymbol = contextMenu.addAction(QAction::tr("Find Usages"));
    QObject::connect(findSymbol, &QAction::triggered, [&](){
        if (getClient()) {
            auto lspPos = getLspPosition(d->edit->docPointer(), d->edit->selectionStart());
            getClient()->referencesRequest(d->edit->file(), lspPos);
        }
    });

    contextMenu.move(showPos);
    contextMenu.exec();
}

void StyleLsp::sciReplaced(const QString &file, Scintilla::Position start, Scintilla::Position end, const QString &text)
{
    if (!d->edit)
        return;
    Q_UNUSED(text)
    Q_UNUSED(start);
    Q_UNUSED(end);
    if (getClient()) {
        getClient()->changeRequest(file, d->edit->textRange(0, d->edit->length()));
    }
}

void StyleLsp::renameRequest(const QString &newText)
{
    auto sciPostion = d->renameCache.getStart().getSciPosition();
    if (d->edit) {
        if (getClient()) {
            getClient()->renameRequest(d->edit->file(),
                                       getLspPosition(d->edit->docPointer(), sciPostion),
                                       newText);
        }
    }
}

StyleLsp::ServerInfo StyleLsp::clientInfoSpec(StyleLsp::ServerInfo info)
{
    return info;
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
    auto client = getClient();
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

void StyleLsp::setDiagnostics(const lsp::DiagnosticsParams &params)
{
    if (!edit())
        return;

    if (params.uri.toLocalFile() != edit()->file())
        return;

    this->cleanDiagnostics();
    for (auto val : params.diagnostics) {
        if (val.severity == lsp::Diagnostic::Severity::Error) { // error
            Sci_Position startPos = getSciPosition(d->edit->docPointer(), val.range.start);
            Sci_Position endPos = getSciPosition(d->edit->docPointer(), val.range.end);
            d->edit->setIndicatorCurrent(INDIC_SQUIGGLE);
            d->edit->indicSetFore(INDIC_SQUIGGLE, StyleColor::color(StyleColor::Table::get()->Red));
            d->edit->indicatorFillRange(startPos, endPos - startPos);

            d->edit->eOLAnnotationSetText(val.range.start.line,"Error: " + val.message.toLatin1());
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
    if (!edit() || edit()->isLeave())
        return;

    if (!d->edit->lexer())
        return;

    if (d->edit->supportLanguage() != d->edit->lexerLanguage()){
        ContextDialog::ok(StyleLsp::tr("There is a fatal error between the current"
                                       " d->editor component and the backend of the syntax server, "
                                       "which may affect the syntax highlighting. \n"
                                       "Please contact the maintainer for troubleshooting "
                                       "to solve the problem!"));
        return;
    }

    int cacheLine = 0;
    for (auto val : tokens) {
        cacheLine += val.start.line;
        /* qInfo() << "line:" << cacheLine;
        qInfo() << "charStart:" << val.start.character;
        qInfo() << "charLength:" << val.length;
        qInfo() << "tokenType:" << val.tokenType;
        qInfo() << "tokenModifiers:" << val.tokenModifiers; */
        auto sciStartPos = StyleLsp::getSciPosition(d->edit->docPointer(), {cacheLine, val.start.character});
        auto sciEndPos = d->edit->wordEndPosition(sciStartPos, true);
        if (sciStartPos != 0 && sciEndPos != d->edit->length()) {
            QString sourceText = d->edit->textRange(sciStartPos, sciEndPos);
            int wordLength = sciEndPos - sciStartPos;
            // qInfo() << "text:" << sourceText;
            // text is word
            if (!sourceText.isEmpty() && wordLength == val.length) {
                QString tokenValue = tokenToDefine(val.tokenType);
                // qInfo() << "tokenValue:" << tokenValue;
                auto indics = symbolIndic(tokenValue, val.tokenModifiers);
                for (int i = 0; i < INDIC_MAX; i++) {
                    if (indics.fore.keys().contains(i)) {
                        // qInfo() << "fillRangeColor:" << hex << indics.fore[i];
                        d->edit->setIndicatorCurrent(i);
                        d->edit->indicSetFlags(i, SC_INDICFLAG_VALUEFORE);
                        d->edit->setIndicatorValue(indics.fore[i]);
                        d->edit->indicatorFillRange(sciStartPos, wordLength);
                    }
                }
            }
        }
    }
}

void StyleLsp::cleanTokenFull()
{
    Q_UNUSED(d->edit);
}

void StyleLsp::setHover(const lsp::Hover &hover)
{
    if (!edit() || edit()->isLeave())
        return;

    d->edit->callTipSetBack(STYLE_DEFAULT);
    if (!hover.contents.value.isEmpty()) {
        d->edit->callTipShow(d->hoverCache.getSciPosition(), hover.contents.value.toUtf8().toStdString().c_str());
    };
    d->hoverCache.clean();
}

void StyleLsp::cleanHover()
{
    d->edit->callTipCancel();
}

void StyleLsp::setDefinition(const lsp::DefinitionProvider &provider)
{
    if (!edit() || edit()->isLeave())
        return;

    d->definitionCache.setProvider(provider);
    auto sciStartPos = d->edit->wordStartPosition(d->definitionCache.getSciPosition(), true);
    auto sciEndPos = d->edit->wordEndPosition(d->definitionCache.getSciPosition(), true);

    if (provider.count() >= 1) {
        d->edit->setIndicatorCurrent(INDIC_COMPOSITIONTHICK);
        d->edit->indicSetFore(INDIC_COMPOSITIONTHICK, d->edit->styleFore(0));
        d->edit->indicatorFillRange(sciStartPos, sciEndPos - sciStartPos);
        if (d->edit->cursor() != 8) {
            d->definitionCache.setCursor(d->edit->cursor());
            d->edit->setCursor(8); // hand from Scintilla platfrom.h
        }
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
