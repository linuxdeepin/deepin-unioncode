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

namespace newlsp{

std::string JsonConvert::addScope(const std::string &src)
{
    return "{" + src + "}";
}

std::string JsonConvert::delScope(const std::string &obj)
{
    return obj.substr(1, obj.size() - 1);
}

std::string JsonConvert::formatKey(const std::string &key)
{
    return "\"" + key + "\"";
}

std::string JsonConvert::formatValue(unsigned int value)
{
    return std::to_string(value);
}

std::string JsonConvert::formatValue(int value)
{
    return std::to_string(value);
}

std::string JsonConvert::formatValue(bool value)
{
    if (true == value)
        return "true";
    if (false == value)
        return "false";
    return "false";
}

std::string JsonConvert::formatValue(const std::string &value)
{
    return "\"" + value + "\"";
}

std::string JsonConvert::formatValue(const std::vector<int> &vecInt)
{
    std::string ret;
    if (vecInt.size() < 0)
        return ret;

    ret += "[";
    for (int i = 0; i < vecInt.size(); i++) {
        ret += formatValue(vecInt[i]);
        if (i < vecInt.size() - 1)
            ret += ",";
    }
    ret += "]";

    return ret;
}

std::string JsonConvert::formatValue(const std::vector<std::string> &vecString)
{
    std::string ret;
    if (vecString.size() < 0)
        return ret;

    ret += "[";
    for (int i = 0; i < vecString.size(); i++) {
        ret += formatValue(vecString[i]);
        if (i < vecString.size() - 1)
            ret += ",";
    }
    ret += "]";

    return ret;
}

std::string JsonConvert::addValue(const std::string &src,
                                  const std::pair<std::string, std::any> &elem)
{
    std::string temp;
    if (elem.first.empty() || !elem.second.has_value())
        return temp;

    temp = formatKey(elem.first) + ":" ;
    if (any_contrast<std::string>(elem.second)) {
        temp += std::any_cast<std::string>(elem.second);
    } else if (any_contrast<int>(elem.second)) {
        temp += std::to_string(std::any_cast<int>(elem.second));
    } else if (any_contrast<unsigned int>(elem.second)) {
        temp += std::to_string(std::any_cast<unsigned int>(elem.second));
    } else if (any_contrast<bool>(elem.second)) {
        temp += std::to_string(std::any_cast<bool>(elem.second));
    }

    if (!src.empty())
        return src + "," + temp;
    else
        return temp;
}

std::string JsonConvert::addValue(const std::string &src,
                                  std::initializer_list<std::pair<std::string, std::any>> &elems)
{
    auto ret = src;
    for (auto elem : elems) {
        ret = addValue(ret, elem);
    }
    return ret;
}

std::string WorkDoneProgressParams::toStdString() const
{
    std::string ret;
    if (workDoneToken) {
        if (any_contrast<bool>(workDoneToken.value()))
            ret = addValue(ret, {"workDoneToken", formatValue(std::any_cast<bool>(workDoneToken.value()))});
        else if (any_contrast<std::string>(workDoneToken.value()))
            ret = addValue(ret, {"workDoneToken", formatValue(std::any_cast<std::string>(workDoneToken.value()))});
    }
    return ret;
}

std::string TextDocumentItem::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"uri", uri});
    ret = addValue(ret, {"languageId", languageId});
    ret = addValue(ret, {"version", version});
    ret = addValue(ret, {"text", text});
    return addScope(ret);
}

std::string VersionedTextDocumentIdentifier::toStdString() const
{
    std::string ret = delScope(TextDocumentIdentifier::toStdString());
    ret = addValue(ret, {"version", version});
    return addScope(ret);
}

std::string TextDocumentIdentifier::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"uri", uri});
    return addScope(ret);
}

std::string OptionalVersionedTextDocumentIdentifier::toStdString() const
{
    std::string ret = delScope(TextDocumentIdentifier::toStdString());
    if (version)
        ret = addValue(ret, {"version", version});
    return addScope(ret);
}

std::string Range::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"start", start});
    ret = addValue(ret, {"end", end});
    return addScope(ret);
}

std::string Position::toStdString() const
{
    std::string ret;
    ret = addValue(ret, {"line", character});
    ret = addValue(ret, {"character", character});
    return addScope(ret);
}

std::string DocumentFilter::toStdString() const
{
    std::string ret;
    if (!language && !scheme && pattern)
        return ret;
    if (language)
        ret = addValue(ret, {"language", language});
    if (scheme)
        ret = addValue(ret, {"scheme", scheme});
    if (scheme)
        ret = addValue(ret, {"pattern", pattern});
    return addScope(ret);
}

std::string DocumentSelector::toStdString() const
{
    std::string ret;
    int size = std::vector<DocumentFilter>::size();
    if (size < 0)
        return ret;

    ret += "[";
    for (int i = 0; i < size; i++) {
        ret += operator[](i).toStdString();
        if (i < size - 1)
            ret += ",";
    }
    ret += "]";
    return ret;
}

} // namespace newlsp
