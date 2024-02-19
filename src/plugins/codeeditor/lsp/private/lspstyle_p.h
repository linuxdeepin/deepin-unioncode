// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LSPSTYLE_P_H
#define LSPSTYLE_P_H

#include "lsp/lspstyle.h"

class SciRangeCache
{
    std::optional<int> start;
    std::optional<int> end;

public:
    SciRangeCache(int start, int end)
        : start(start), end(end) {}
    SciRangeCache() {}
    void clean()
    {
        start.reset();
        end.reset();
    }
    bool isEmpty() { return start && end; }
    int getStart() const { return start.value(); }
    void setStart(int value) { start = value; }
    int getEnd() const { return end.value(); }
    void setEnd(int value) { end = value; }
    bool operator==(const SciRangeCache &other)
    {
        return start == other.start && end == other.end;
    }
};

class SciPositionCache
{
    int sciPosition = -1;

public:
    SciPositionCache() {}
    SciPositionCache(int pos)
        : sciPosition(pos) {}
    void clean() { sciPosition = -1; }
    bool isEmpty() { return sciPosition == -1; }
    int getSciPosition() const { return sciPosition; }
    void setSciPosition(int value) { sciPosition = value; }
};

class DefinitionCache : public SciPositionCache
{
    std::optional<std::vector<newlsp::Location>> locations {};
    std::optional<std::vector<newlsp::LocationLink>> locationLinks {};
    std::optional<newlsp::Location> location {};
    SciRangeCache textRange {};
    int cursor = 0;   //Invalid
public:
    void clean()
    {
        cleanFromLsp();
        cursor = 0;
        SciPositionCache::clean();
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
    newlsp::Location getLocation() const { return location.value(); }
    std::vector<newlsp::LocationLink> getLocationLinks() const { return locationLinks.value(); }
    void set(const std::vector<newlsp::Location> &value) { locations = value; }
    void set(const newlsp::Location &value) { location = value; }
    void set(const std::vector<newlsp::LocationLink> &value) { locationLinks = value; }
    int getCursor() const { return cursor; }
    void setCursor(int value) { cursor = value; }
    SciRangeCache getTextRange() const { return textRange; }
    void setTextRange(const SciRangeCache &value) { textRange = value; }
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
    void clean()
    {
        start.clean();
        end.clean();
    }
    bool isEmpty() { return start.isEmpty() && end.isEmpty(); }
    SciPositionCache getStart() const { return start; }
    void setStart(const SciPositionCache &value) { start = value; }
    SciPositionCache getEnd() const { return end; }
    void setEnd(const SciPositionCache &value) { end = value; }
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

class LSPStylePrivate
{
public:
    CompletionCache completionCache;
    DefinitionCache definitionCache;
    QTimer textChangedTimer;
    HoverCache hoverCache;
//    RenamePopup renamePopup;
    RenameCache renameCache;
    TextEditor *editor { nullptr };
    TextChangeCache textChangedCache;
    QList<lsp::Data> tokensCache;
    static QAction *rangeFormattingAction;
    static QString formattingFile;
    friend class StyleLsp;
    newlsp::Client *getClient() const;
};

#endif   // LSPSTYLE_P_H
