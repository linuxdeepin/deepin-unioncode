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

public:
    PositionCache() {}
    PositionCache(int pos)
        : position(pos) {}
    void clean() { position = -1; }
    bool isEmpty() { return position == -1; }
    int getPosition() const { return position; }
    void setPosition(int value) { position = value; }
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
        cursor = 0;
        PositionCache::clean();
        textRange.clean();
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
                && cursor == 0 && PositionCache::isEmpty()
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

struct TextChangeCache
{
    enum State {
        Inserted,
        Deleted
    };
    State state;
    int positionCache;
    int lengthCache;
    int linesAddedCache;
    QString textCache;
    int lineCache;
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

class LSPStylePrivate
{
public:
    QString formatDiagnosticMessage(const QString &message, int type);

    CompletionCache completionCache;
    DefinitionCache definitionCache;
    QTimer textChangedTimer;
    HoverCache hoverCache;
    RenamePopup renamePopup;
    RenamePositionCache renameCache;
    TextEditor *editor { nullptr };
    TextChangeCache textChangedCache;
    QList<lsp::Data> tokensCache;
    QList<DiagnosticCache> diagnosticCache;
    static QAction *rangeFormattingAction;
    static QString formattingFile;
    friend class StyleLsp;
    newlsp::Client *getClient() const;
    QString diagnosticFormat;
};

#endif   // LSPSTYLE_P_H
