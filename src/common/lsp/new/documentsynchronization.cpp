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
std::string toJsonValueStr(const DidOpenTextDocumentParams &val)
{

}

std::string toJsonValueStr(const DidChangeTextDocumentParams &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"contentChanges", val.contentChanges});
    return json::addScope(ret);
}

std::string toJsonValueStr(const std::vector<TextDocumentContentChangeEvent> &val)
{
    std::string ret;
    if (val.size() < 0)
        return ret;

    ret += "[";
    for (int i = 0; i < val.size(); i++) {
        ret += toJsonValueStr(val[i]);
        if (i < val.size() - 1)
            ret += ",";
    }
    ret += "]";
    return ret;
}

std::string toJsonValueStr(const TextDocumentContentChangeEvent &val)
{
    std::string ret;
    if (val.range)
        ret = json::addValue(ret, json::KV{"range", val.range});
    if (val.rangeLength)
        ret = json::addValue(ret, json::KV{"rangeLength", val.rangeLength});
    ret = json::addValue(ret , json::KV{"text", val.text});
    return json::addScope(ret);
}

std::string toJsonValueStr(const WillSaveTextDocumentParams &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"reason", val.reason});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SaveOptions &val)
{
    std::string ret;
    if (val.includeText)
        ret = json::addValue(ret, json::KV{"includeText", val.includeText});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TextDocumentRegistrationOptions &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"documentSelector", val.documentSelector});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TextDocumentSaveRegistrationOptions &val)
{
    //= json::delScope(TextDocumentRegistrationOptions::toJsonValueStr());
    std::string ret;
    if (val.includeText)
        ret = json::addValue(ret, json::KV{"includeText", val.includeText});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DidSaveTextDocumentParams &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    if (val.text)
        ret = json::addValue(ret, json::KV{"text", val.text});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DidCloseTextDocumentParams &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TextDocumentSyncOptions &val)
{
    std::string ret;
    if (val.openClose)
        ret = json::addValue(ret, json::KV{"openColse", val.openClose});
    if (val.change)
        ret = json::addValue(ret, json::KV{"change", val.change});
    if (val.willSave)
        ret = json::addValue(ret, json::KV{"willSave", val.willSave});
    if (val.willSaveWaitUntil)
        ret = json::addValue(ret, json::KV{"willSaveWaitUntil", val.willSaveWaitUntil});
    if (val.save) {
        if (any_contrast<SaveOptions>(val.save))
            ret = json::addValue(ret, json::KV{"save", std::any_cast<SaveOptions>(val.save)});
        if (any_contrast<bool>(val.save))
            ret = json::addValue(ret, json::KV{"save", std::any_cast<bool>(val.save)});
    }
    return json::addScope(ret);
}
} // namespace newlsp
