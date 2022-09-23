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
#include "documentsynchronization.h"

namespace newlsp {

std::string DidOpenTextDocumentParams::toStdString() const
{
    std::string res;
    addValue(res, {"textDocument", textDocument.toStdString()});
    return addScope(res);
}

std::string DidChangeTextDocumentParams::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"textDocument", textDocument});
    ret = addValue(ret, {"contentChanges", formatValue(contentChanges)});
    return addScope(ret);
}

std::string DidChangeTextDocumentParams::formatValue(const std::vector<TextDocumentContentChangeEvent> &val) const
{
    std::string ret;
    if (val.size() < 0)
        return ret;

    ret += "[";
    for (int i = 0; i < val.size(); i++) {
        ret += val[i].toStdString();
        if (i < val.size() - 1)
            ret += ",";
    }
    ret += "]";
    return ret;
}

std::string TextDocumentContentChangeEvent::toStdString() const
{
    std::string ret;
    if (range)
        ret = addValue(ret, {"range", range->toStdString()});
    if (rangeLength)
        ret = addValue(ret, {"rangeLength", rangeLength});
    ret = addValue(ret , {"text", text});
    return addScope(ret);
}

std::string WillSaveTextDocumentParams::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"textDocument", textDocument.toStdString()});
    ret = addValue(ret, {"reason", reason});
    return addScope(ret);
}

std::string SaveOptions::toStdString() const
{
    std::string ret;
    if (includeText)
        ret = addValue(ret, {"includeText", includeText});
    return addScope(ret);
}

std::string TextDocumentRegistrationOptions::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"documentSelector", documentSelector.toStdString()});
    return addScope(ret);
}

std::string TextDocumentSaveRegistrationOptions::toStdString() const
{
    std::string ret = delScope(TextDocumentRegistrationOptions::toStdString());
    if (includeText)
        ret = addValue(ret, {"includeText", includeText});
    return addScope(ret);
}

std::string DidSaveTextDocumentParams::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"textDocument", textDocument.toStdString()});
    if (text)
        ret = addValue(ret, {"text", text});
    return addScope(ret);
}

std::string DidCloseTextDocumentParams::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"textDocument", textDocument});
    return addScope(ret);
}

std::string TextDocumentSyncOptions::toStdString() const
{
    std::string ret;
    if (openClose)
        ret = addValue(ret, {"openColse", openClose});
    if (change)
        ret = addValue(ret, {"change", change});
    if (willSave)
        ret = addValue(ret, {"willSave", willSave});
    if (willSaveWaitUntil)
        ret = addValue(ret, {"willSaveWaitUntil", willSaveWaitUntil});
    if (save) {
        if (any_contrast<SaveOptions>(save))
            ret = addValue(ret, {"save", std::any_cast<SaveOptions>(save).toStdString()});
        if (any_contrast<bool>(save))
            ret = addValue(ret, {"save", save});
    }
    return addScope(ret);
}

} // namespace newlsp
