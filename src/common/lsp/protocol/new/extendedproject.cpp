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
#include "extendedproject.h"

#include "common/lsp/protocol/new/basicjsonstructures.h"

#include <QMetaType>
#include <QHash>

namespace newlsp
{

uint qHash(const ProjectKey &key, uint seed)
{
    return ::qHash(QString::fromStdString(key.workspace)
                   + QString::fromStdString(key.language), seed);
}

bool operator ==(const ProjectKey &t1, const ProjectKey &t2)
{
    return t1.workspace == t2.workspace
            && t2.language == t2.language;
}

std::string toJsonValueStr(const ProjectKey &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"language", val.language});
    ret = json::addValue(ret, json::KV{"workspace", val.workspace});
    return json::addScope(ret);
}

std::string toJsonValueStr(const LanuchLspServerParams &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"projectKey", val.projectKey});
    return json::addScope(ret);
}

std::string toJsonValueStr(const SelectLspServerParams &val)
{
    std::string ret;
    ret = json::addValue(ret, json::KV{"projectKey", val.projectKey});
    return json::addScope(ret);
}

QJsonObject toQJsonObject(const ProjectKey &val)
{
    QJsonObject ret;
    ret["language"] = QString::fromStdString(val.language);
    ret["workspace"] = QString::fromStdString(val.workspace);
    return ret;
}

ProjectKey::ProjectKey()
{
    qRegisterMetaType<newlsp::ProjectKey>("newlsp::ProjectKey");
}

ProjectKey::ProjectKey(const std::string &language, const std::string &workspace)
    : language(language), workspace(workspace)
{
    qRegisterMetaType<newlsp::ProjectKey>("newlsp::ProjectKey");
}

ProjectKey::ProjectKey(const ProjectKey &other)
    : language(other.language), workspace(other.workspace)
{
    qRegisterMetaType<newlsp::ProjectKey>("newlsp::ProjectKey");
}

}
