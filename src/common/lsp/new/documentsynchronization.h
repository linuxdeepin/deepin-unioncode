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

namespace newlsp
{

struct DidOpenTextDocumentParams : JsonConvert
{
    TextDocumentItem textDocument;
    std::string toStdString() const;
};

struct TextDocumentContentChangeEvent : JsonConvert
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
    std::string toStdString() const;
};

struct DidChangeTextDocumentParams : JsonConvert
{
    VersionedTextDocumentIdentifier textDocument;
    std::vector<TextDocumentContentChangeEvent> contentChanges;
    std::string formatValue(const std::vector<TextDocumentContentChangeEvent> &val) const;
    std::string toStdString() const;
};

enum_def(TextDocumentSaveReason, int)
{
    enum_exp Manual = 1;
    enum_exp AfterDelay = 2;
    enum_exp FocusOut = 3;
};

struct WillSaveTextDocumentParams : JsonConvert
{
    TextDocumentIdentifier textDocument;
    TextDocumentSaveReason::type_value reason;
    std::string toStdString() const;
};

struct SaveOptions : JsonConvert
{
    std::optional<bool> includeText;
    std::string toStdString() const;
};

struct TextDocumentRegistrationOptions : JsonConvert
{
    DocumentSelector documentSelector;
    std::string toStdString() const;
};

struct TextDocumentSaveRegistrationOptions : TextDocumentRegistrationOptions
{
    std::optional<bool> includeText;
    std::string toStdString() const;
};

struct DidSaveTextDocumentParams : JsonConvert
{
    TextDocumentIdentifier textDocument;
    std::optional<std::string> text;
    std::string toStdString() const;
};

struct DidCloseTextDocumentParams : JsonConvert
{
    TextDocumentIdentifier textDocument;
    std::string toStdString() const;
};

enum_def(TextDocumentSyncKind, int)
{
    enum_exp None = 0;
    enum_exp Full = 1;
    enum_exp Incremental = 2;
};

struct TextDocumentSyncOptions : JsonConvert
{
    std::optional<bool> openClose;
    std::optional<TextDocumentSyncKind::type_value> change;
    std::optional<bool> willSave;
    std::optional<bool> willSaveWaitUntil;
    std::optional<std::any> save; // boolean | SaveOptions
    std::string toStdString() const;
};

}

#endif // DOCUMENTSYNCHRONIZATION_H
