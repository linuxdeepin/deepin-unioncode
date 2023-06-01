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
#include "basicjsonstructures.h"

#include <QJsonDocument>
#include <QJsonObject>

namespace newlsp{

const QString K_ID {"id"};
const QString K_JSON_RPC {"jsonrpc"};
const QString V_2_0 {"2.0"};
const QString K_METHOD {"method"};
const QString K_RESULT {"result"};
const QString K_PARAMS {"params"};
const QString H_CONTENT_LENGTH {"Content-Length"};
const QString H_CONTENT_TYPE {"Content-Type"};
const QString H_CHARSET {"charset"};
const QString RK_CONTENT_LENGTH {"ContentLength"};
const QString RK_CONTENT_TYPE {"ContentType"};
const QString RK_CHARSET {"charset"};

std::string toJsonValueStr(unsigned int value)
{
    return std::to_string(value);
}

std::string toJsonValueStr(int value)
{
    return std::to_string(value);
}

std::string toJsonValueStr(bool value)
{
    if (true == value)
        return "true";
    if (false == value)
        return "false";
    return "false";
}

std::string toJsonValueStr(float value)
{
    return std::to_string(value);
}

std::string toJsonValueStr(double value)
{
    return  std::to_string(value);
}

std::string toJsonValueStr(const std::string &value)
{
    return "\"" + value + "\"";
}

std::string toJsonValueStr(const std::vector<int> &vecInt)
{
    std::string ret;
    if (vecInt.size() < 0)
        return ret;

    ret += "[";
    int vecSize = vecInt.size();
    for (int i = 0; i < vecSize; i++) {
        ret += toJsonValueStr(vecInt[i]);
        if (i < vecSize - 1)
            ret += ",";
    }
    ret += "]";

    return ret;
}

std::string toJsonValueStr(const std::vector<std::string> &vecString)
{
    std::string ret;
    if (vecString.size() < 0)
        return ret;

    ret += "[";
    int vecSize = vecString.size();
    for (int i = 0; i < vecSize; i++) {
        ret += toJsonValueStr(vecString[i]);
        if (i < vecSize - 1)
            ret += ",";
    }
    ret += "]";

    return ret;
}

std::string json::addScope(const std::string &src)
{
    return "{" + src + "}";
}

std::string json::delScope(const std::string &obj)
{
    std::string ret = obj;
    ret.erase(0, 1);
    ret.erase(ret.size() - 1, ret.size());
    return ret;
}

std::string json::formatKey(const std::string &key)
{
    return "\"" + key + "\"";
}

std::string toJsonValueStr(const ProgressToken &val)
{
    if (any_contrast<int>(val))
        return toJsonValueStr(std::any_cast<int>(val));
    else if (any_contrast<std::string>(val))
        return toJsonValueStr(std::any_cast<std::string>(val));
    return "{}";
}

std::string toJsonValueStr(const TextDocumentItem &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"uri", val.uri});
    ret = json::addValue(ret, json::KV{"languageId", val.languageId});
    ret = json::addValue(ret, json::KV{"version", val.version});
    ret = json::addValue(ret, json::KV{"text", val.text});
    return json::addScope(ret);
}

std::string toJsonValueStr(const VersionedTextDocumentIdentifier &val)
{
    std::string ret = json::delScope(toJsonValueStr(TextDocumentIdentifier(val)));
    ret = json::addValue(ret, json::KV{"version", val.version});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TextDocumentIdentifier &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"uri", val.uri});
    return json::addScope(ret);
}

std::string toJsonValueStr(const OptionalVersionedTextDocumentIdentifier &val)
{
    std::string ret = json::delScope(toJsonValueStr(TextDocumentIdentifier(val)));
    ret = json::addValue(ret, json::KV{"version", val.version});
    return json::addScope(ret);
}

std::string toJsonValueStr(const Range &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"start", val.start});
    ret = json::addValue(ret, json::KV{"end", val.end});
    return json::addScope(ret);
}

std::string toJsonValueStr(const Position &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"line", val.line});
    ret = json::addValue(ret, json::KV{"character", val.character});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentFilter &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"language", val.language});
    ret = json::addValue(ret, json::KV{"scheme", val.scheme});
    ret = json::addValue(ret, json::KV{"pattern", val.pattern});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DocumentSelector &val)
{
    std::string ret;
    int size = val.size();
    if (size < 0)
        return ret;

    ret += "[";
    for (int i = 0; i < size; i++) {
        ret += toJsonValueStr(val[i]);
        if (i < size - 1)
            ret += ",";
    }
    ret += "]";
    return ret;
}

std::string toJsonValueStr(const WorkDoneProgressOptions &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"workDoneProgress", val.workDoneProgress});
    return json::addScope(ret);
}

std::string toJsonValueStr(const WorkDoneProgressParams &params)
{
    std::string ret;
    if (params.workDoneToken) {
        if (any_contrast<bool>(params.workDoneToken)) {
            bool tv = std::any_cast<bool>(params.workDoneToken);
            ret = json::addValue(ret, json::KV{"workDoneToken", tv});
        } else if (any_contrast<std::string>(params.workDoneToken)) {
            std::string tv = std::any_cast<std::string>(params.workDoneToken);
            ret = json::addValue(ret, json::KV{"workDoneToken", tv});
        }
    }
    return json::addScope(ret);
}

std::string toJsonValueStr(const TextDocumentPositionParams &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"position", val.position});
    return json::addScope(ret);
}

std::string json::mergeObjs(const std::vector<std::string> &objs)
{
    std::string ret;
    auto itera = objs.begin();
    while (itera != objs.end()) {
        if (!ret.empty())
            ret += ",";
        ret += json::delScope(*itera);
        itera ++;
    }
    return json::addScope(ret);
}

std::string toJsonValueStr(const PartialResultParams &params)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"partialResultToken", params.partialResultToken});
    return json::addScope(ret);
}

std::string toJsonValueStr(const Command &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"title", val.title});
    ret = json::addValue(ret, json::KV{"command", val.command});
    ret = json::addValue(ret, json::KV{"arguments", val.arguments});
    return json::addScope(ret);
}

std::string toJsonValueStr(const Diagnostic &val)
{
    std::string ret;
    if (any_contrast<int>(val.code))
        ret = json::addValue(ret, json::KV{"title", std::any_cast<int>(val.code)});
    else if (any_contrast<std::string>(val.code))
        ret = json::addValue(ret, json::KV{"title", std::any_cast<std::string>(val.code)});
    ret = json::addValue(ret, json::KV{"data", val.data});
    ret = json::addValue(ret, json::KV{"tags", val.tags});
    ret = json::addValue(ret, json::KV{"range", val.range});
    ret = json::addValue(ret, json::KV{"source", val.source});
    ret = json::addValue(ret, json::KV{"message", val.message});
    ret = json::addValue(ret, json::KV{"severity", val.severity});
    ret = json::addValue(ret, json::KV{"codeDescription", val.codeDescription});
    ret = json::addValue(ret, json::KV{"relatedInformation", val.relatedInformation});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CodeDescription &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"href", val.href});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DiagnosticRelatedInformation &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"message", val.message});
    ret = json::addValue(ret, json::KV{"location", val.location});
    return json::addScope(ret);
}

std::string toJsonValueStr(const Location &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"uri", val.uri});
    ret = json::addValue(ret, json::KV{"range", val.range});
    return json::addScope(ret);
}

std::string toJsonValueStr(const WorkspaceEdit &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"changes", val.changes});
    ret = json::addValue(ret, json::KV{"documentChanges", val.documentChanges});
    ret = json::addValue(ret, json::KV{"changeAnnotations", val.changeAnnotations});
    return json::addScope(ret);
}

std::string toJsonValueStr(const WorkspaceEdit::Changes &val)
{
    std::string ret;
    for (auto &&one : val) {
        ret = json::addValue(ret, json::KV{one.first, one.second});
    }
    return json::addScope(ret);
}

std::string toJsonValueStr(const WorkspaceEdit::ChangeAnnotations &val)
{
    std::string ret;
    for (auto &&one : val) {
        ret = json::addValue(ret, json::KV{one.first, one.second});
    }
    return json::addScope(ret);
}

std::string toJsonValueStr(const ChangeAnnotation &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"label", val.label});
    ret = json::addValue(ret, json::KV{"description", val.description});
    ret = json::addValue(ret, json::KV{"needsConfirmation", val.needsConfirmation});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DeleteFile &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"kind", val.kind});
    ret = json::addValue(ret, json::KV{"uri", val.uri});
    ret = json::addValue(ret, json::KV{"options", val.options});
    ret = json::addValue(ret, json::KV{"annotationId", val.annotationId});
    return json::addScope(ret);
}

std::string toJsonValueStr(const DeleteFileOptions &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"recursive", val.recursive});
    ret = json::addValue(ret, json::KV{"ignoreIfNotExists", val.ignoreIfNotExists});
    return json::addScope(ret);
}

std::string toJsonValueStr(const RenameFile &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"kind", val.kind});
    ret = json::addValue(ret, json::KV{"oldUri", val.oldUri});
    ret = json::addValue(ret, json::KV{"newUri", val.newUri});
    ret = json::addValue(ret, json::KV{"options", val.options});
    ret = json::addValue(ret, json::KV{"annotationId", val.annotationId});
    return json::addScope(ret);
}

std::string toJsonValueStr(const RenameFileOptions &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"overwrite", val.overwrite});
    ret = json::addValue(ret, json::KV{"ignoreIfExists", val.ignoreIfExists});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CreateFile &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"kind", val.kind});
    ret = json::addValue(ret, json::KV{"uri", val.uri});
    ret = json::addValue(ret, json::KV{"options", val.options});
    ret = json::addValue(ret, json::KV{"annotationId", val.annotationId});
    return json::addScope(ret);
}

std::string toJsonValueStr(const CreateFileOptions &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"overwrite", val.overwrite});
    ret = json::addValue(ret, json::KV{"ignoreIfExists", val.ignoreIfExists});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TextDocumentEdit &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"textDocument", val.textDocument});
    ret = json::addValue(ret, json::KV{"edits", val.edits});
    return json::addScope(ret);
}

std::string toJsonValueStr(const AnnotatedTextEdit &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"annotationId", val.annotationId});
    return json::addScope(ret);
}

std::string toJsonValueStr(const TextEdit &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"range", val.range});
    ret = json::addValue(ret, json::KV{"newText", val.newText});
    return json::addScope(ret);
}

std::string toJsonValueStr(const WorkspaceEdit::DocumentChanges &val)
{
    std::string ret;
    if (any_contrast<std::vector<TextDocumentEdit>>(val))
        ret = toJsonValueStr(std::any_cast<std::vector<TextDocumentEdit>>(val));
    else if (any_contrast<std::vector<CreateFile>>(val))
        ret = toJsonValueStr(std::any_cast<CreateFile>(val));
    else if (any_contrast<std::vector<RenameFile>>(val))
        ret = toJsonValueStr(std::any_cast<RenameFile>(val));
    else if (any_contrast<std::vector<DeleteFile>>(val))
        ret = toJsonValueStr(std::any_cast<std::vector<DeleteFile>>(val));
    return ret;
}

std::string toJsonValueStr(const TextDocumentEdit::Edits &val)
{
    std::string ret;
    if (!std::vector<AnnotatedTextEdit>(val).empty()) {
        auto textEdit = std::vector<AnnotatedTextEdit>(val);
        int size = textEdit.size();
        if (size < 0)
            return ret;

        ret += "[";
        for (int i = 0; i < size; i++) {
            ret += toJsonValueStr(textEdit[i]);
            if (i < size - 1)
                ret += ",";
        }
        ret += "]";
        return ret;
    }
    else if (!std::vector<TextEdit>(val).empty()) {
        auto textEdit = std::vector<TextEdit>(val);
        int size = textEdit.size();
        if (size < 0)
            return ret;

        ret += "[";
        for (int i = 0; i < size; i++) {
            ret += toJsonValueStr(textEdit[i]);
            if (i < size - 1)
                ret += ",";
        }
        ret += "]";
        return ret;
    }
    return json::addScope(ret);
}

QString methodData(int id, const QString &method, const QJsonObject &params)
{
    QJsonObject retObj;
    retObj[K_JSON_RPC] = V_2_0;
    retObj[K_PARAMS] = params;
    retObj[K_ID] = id;
    retObj[K_METHOD] = method;
    QJsonDocument jsonDoc(retObj);
    QString jsonStr = jsonDoc.toJson(QJsonDocument::JsonFormat::Compact);
    return H_CONTENT_LENGTH + QString(": %0\r\n\r\n").arg(jsonStr.length()) + jsonStr + "\n";
}

QString notificationData(const QString &method, const QJsonObject &params)
{
    QJsonObject retObj;
    retObj[K_JSON_RPC] = V_2_0;
    retObj[K_PARAMS] = params;
    retObj[K_METHOD] = method;
    QJsonDocument jsonDoc(retObj);
    QString jsonStr = jsonDoc.toJson(QJsonDocument::JsonFormat::Compact);
    return H_CONTENT_LENGTH + QString(": %0\r\n\r\n").arg(jsonStr.length()) + jsonStr + "\n";
}

} // namespace newlsp
