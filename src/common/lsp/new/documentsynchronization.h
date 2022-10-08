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
#ifndef DOCUMENTSYNCHRONIZATION_H
#define DOCUMENTSYNCHRONIZATION_H

#include "basicjsonstructures.h"

namespace newlsp {
struct DidOpenTextDocumentParams
{
    TextDocumentItem textDocument;
};
std::string toJsonValueStr(const DidOpenTextDocumentParams &val);

struct TextDocumentContentChangeEvent
{
    std::optional<Range> range;
    std::optional<unsigned int> rangeLength;
    std::string text;
    TextDocumentContentChangeEvent(const TextDocumentContentChangeEvent &other)
    {
        if (other.range)
            range = other.range;
        if (other.rangeLength)
            rangeLength = other.rangeLength;
        text = other.text;
    }
};
std::string toJsonValueStr(const TextDocumentContentChangeEvent &val);

struct DidChangeTextDocumentParams
{
    VersionedTextDocumentIdentifier textDocument;
    std::vector<TextDocumentContentChangeEvent> contentChanges;
    std::string formatValue(const std::vector<TextDocumentContentChangeEvent> &val) const;
};
std::string toJsonValueStr(const std::vector<TextDocumentContentChangeEvent> &val);

struct WillSaveTextDocumentParams
{
    TextDocumentIdentifier textDocument;
    newlsp::Enum::TextDocumentSaveReason::type_value reason;
};

struct SaveOptions
{
    std::optional<bool> includeText;
};

struct TextDocumentRegistrationOptions
{
    DocumentSelector documentSelector;
};
std::string toJsonValueStr(const TextDocumentRegistrationOptions &val);

struct TextDocumentSaveRegistrationOptions : TextDocumentRegistrationOptions
{
    std::optional<bool> includeText;
};

struct DidSaveTextDocumentParams
{
    TextDocumentIdentifier textDocument;
    std::optional<std::string> text;
};

struct DidCloseTextDocumentParams
{
    TextDocumentIdentifier textDocument;
};

struct TextDocumentSyncOptions
{
    std::optional<bool> openClose;
    std::optional<newlsp::Enum::TextDocumentSyncKind::type_value> change;
    std::optional<bool> willSave;
    std::optional<bool> willSaveWaitUntil;
    std::optional<std::any> save; // boolean | SaveOptions
};

struct ExecutionSummary
{
    unsigned int executionOrder;
    bool success;
};

struct NotebookCell
{
    Enum::NotebookCellKind::type_value kind;
    DocumentUri document;
    std::any metadata;
    ExecutionSummary executionSummary;
};

struct NotebookDocument
{
    URI uri;
    std::string notebookType;
    int version;
    std::any metadata; //export type LSPObject = { [key: string]: LSPAny };
    std::vector<NotebookCell> cells;
};

/* *
 * Many structural encapsulation of notebook are omitted here,
 * which is not used temporarily
 * */
} // newlsp

#endif // DOCUMENTSYNCHRONIZATION_H
