// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "documentsynchronization.h"

namespace newlsp {
std::string toJsonValueStr(const DidOpenTextDocumentParams &val)
{
    return {};
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
    int n = val.size();
    for (int i = 0; i < n; i++) {
        ret += toJsonValueStr(val[i]);
        if (i < n - 1)
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
