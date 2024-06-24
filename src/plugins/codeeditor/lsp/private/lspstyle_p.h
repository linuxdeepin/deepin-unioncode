// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LSPSTYLE_P_H
#define LSPSTYLE_P_H

#include "lsp/lspstyle.h"
#include "renamepopup/renamepopup.h"

class RangeCache
{
    int start = 0;
    int end = 0;

public:
    RangeCache(int start, int end)
        : start(start), end(end) {}
    RangeCache() {}
    void clean()
    {
        start = 0;
        end = 0;
    }
    bool isEmpty() { return !(start && end); }
    bool contaions(int pos) { return pos >= start && pos <= end; }
    int getStart() const { return start; }
    void setStart(int value) { start = value; }
    int getEnd() const { return end; }
    void setEnd(int value) { end = value; }
    bool operator==(const RangeCache &other)
    {
        return start == other.start && end == other.end;
    }
    bool operator!=(const RangeCache &other)
    {
        return !(operator==(other));
    }
};

class PositionCache
{
    int position = -1;
    RangeCache textRange;

public:
    PositionCache() {}
    PositionCache(int pos)
        : position(pos) {}
    void clean()
    {
        position = -1;
        textRange.clean();
    }
    bool isEmpty() { return position == -1 || textRange.isEmpty(); }
    int getPosition() const { return position; }
    void setPosition(int value) { position = value; }
    RangeCache getTextRange() const { return textRange; }
    void setTextRange(int start, int end)
    {
        textRange.setStart(start);
        textRange.setEnd(end);
    }
};

class DefinitionCache : public PositionCache
{
public:
    enum SwitchMode {
        ActionMode,   // menu
        ClickMode   // click with Ctrl
    };

    void clean()
    {
        cleanFromLsp();
        cursor = -1;
        PositionCache::clean();
        textRange.clean();
        mode = ClickMode;
    }
    void cleanFromLsp()
    {
        if (locations) location.reset();
        if (location) location.reset();
        if (locationLinks) locationLinks.reset();
    }

    bool isEmpty()
    {
        return locations->empty() && !location.has_value() && locationLinks->empty()
                && cursor == -1 && PositionCache::isEmpty()
                && textRange.isEmpty();
    }
    std::vector<newlsp::Location> getLocations() const
    {
        if (locations.has_value())
            return locations.value();
        return {};
    }
    newlsp::Location getLocation() const { return location.value(); }
    std::vector<newlsp::LocationLink> getLocationLinks() const { return locationLinks.value(); }
    void set(const std::vector<newlsp::Location> &value) { locations = value; }
    void set(const newlsp::Location &value) { location = value; }
    void set(const std::vector<newlsp::LocationLink> &value) { locationLinks = value; }
    int getCursor() const { return cursor; }
    void setCursor(int value) { cursor = value; }
    RangeCache getTextRange() const { return textRange; }
    void setTextRange(const RangeCache &value) { textRange = value; }
    void setSwitchMode(SwitchMode mode) { this->mode = mode; }
    SwitchMode switchMode() const { return mode; }

private:
    std::optional<std::vector<newlsp::Location>> locations {};
    std::optional<std::vector<newlsp::LocationLink>> locationLinks {};
    std::optional<newlsp::Location> location {};
    RangeCache textRange {};
    int cursor = 0;   //Invalid
    SwitchMode mode { ClickMode };
};

class HoverCache : public PositionCache
{
public:
    void clean() { PositionCache::clean(); }
    bool isEmpty() { return PositionCache::isEmpty(); }
};

class CompletionCache
{
public:
    lsp::CompletionProvider provider;
};

struct RenamePositionCache
{
    int line = -1;
    int column = -1;

    void clear()
    {
        line = -1;
        column = -1;
    }

    bool isValid() { return line != -1 && column != -1; }
};

struct DiagnosticCache
{
    int startPos = 0;
    int endPos = 0;
    QString message;
    int type;

public:
    bool contains(int pos) const
    {
        return (pos >= startPos && pos <= endPos);
    }
};

class LSPStylePrivate : public QObject
{
    Q_OBJECT
public:
    explicit LSPStylePrivate(TextEditor *edit, LSPStyle *qq);

    void initConnection();
    void initLspConnection();
    void initIndicStyle();

    QString formatDiagnosticMessage(const QString &message, int type);
    bool shouldStartCompletion(const QString &insertedText);
    int wordPostion();
    newlsp::Client *getClient();

    QColor symbolIndicColor(lsp::SemanticTokenType::type_value token,
                            QList<lsp::SemanticTokenType::type_index> modifier);
    lsp::SemanticTokenType::type_value tokenToDefine(int token);
    void cleanDiagnostics();
    void cleanDefinition(int pos);
    void setDefinitionSelectedStyle(int start, int end);
    void gotoDefinition();

public slots:
    void handleTokenFull(const QList<lsp::Data> &tokens, const QString &filePath);
    void handleShowHoverInfo(const newlsp::Hover &hover);
    void handleCodeDefinition(const newlsp::Location &data, const QString &filePath);
    void handleCodeDefinition(const std::vector<newlsp::Location> &data, const QString &filePath);
    void handleCodeDefinition(const std::vector<newlsp::LocationLink> &data, const QString &filePath);
    void handleDiagnostics(const newlsp::PublishDiagnosticsParams &data);
    void handleRangeFormattingReplace(const std::vector<newlsp::TextEdit> &edits, const QString &filePath);
    void delayTextChanged();
    void delayPositionChanged();
    void handleHoveredStart(int position);
    void handleHoverEnd(int position);
    void handleFollowTypeStart(int position);
    void handleFollowTypeEnd();
    void handleIndicClicked(int line, int index);
    void handleShowContextMenu(QMenu *menu);
    void handleFileClosed(const QString &file);
    void handleRename(const QString &text);
    void handleSwitchHeaderSource(const QString &file);
    void handleDocumentSymbolResult(const QList<newlsp::DocumentSymbol> &docSymbols, const QString &filePath);
    void handleSymbolInfomationResult(const QList<newlsp::SymbolInformation> &symbolInfos, const QString &filePath);
    void handleDocumentHighlight(const QList<newlsp::DocumentHighlight> &docHighlightList, const QString &filePath);

public:
    LSPStyle *q;

    CompletionCache completionCache;
    DefinitionCache definitionCache;
    HoverCache hoverCache;
    RenamePopup renamePopup;
    RenamePositionCache renameCache;
    TextEditor *editor { nullptr };
    QList<lsp::Data> tokensCache;
    QList<DiagnosticCache> diagnosticCache;
    QString diagnosticFormat;
    newlsp::ProjectKey prjectKey;
    QList<newlsp::SymbolInformation> symbolInfoList;
    QList<newlsp::DocumentSymbol> docSymbolList;

    QTimer textChangedTimer;
    QTimer positionChangedTimer;

    friend class StyleLsp;
};

#endif   // LSPSTYLE_P_H
