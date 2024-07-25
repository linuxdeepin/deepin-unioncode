// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENDEDPROJECT_H
#define EXTENDEDPROJECT_H

#include <QJsonObject>
#include <string>

namespace newlsp
{

inline const std::string Cxx{"C/C++"};
inline const std::string Java{"Java"};
inline const std::string Python{"Python"};
inline const std::string JS{"JS"};

inline const std::string language{"language"};
inline const std::string workspace{"workspace"};

inline const std::string lauchLspServer{"lanuchLspServer"};
inline const std::string selectLspServer{"selectLspServer"};

struct ProjectKey
{
    std::string language;
    std::string workspace;
    ProjectKey();
    ProjectKey(const std::string &language, const std::string &workspace);
    ProjectKey(const ProjectKey &other);
    bool isValid() const { return !workspace.empty() && !language.empty();}
    bool operator == (const ProjectKey &other) {
        return language == other.language
                && workspace == other.workspace;
    }
};

uint qHash(const ProjectKey &key, uint seed = 0);
bool operator == (const ProjectKey &t1, const ProjectKey &t2);
std::string toJsonValueStr(const ProjectKey &val);
QJsonObject toQJsonObject(const ProjectKey &val);

/* request */
struct LanuchLspServerParams
{
    ProjectKey projectKey;
};
std::string toJsonValueStr(const LanuchLspServerParams &val);

/* notification */
struct SelectLspServerParams
{
    ProjectKey projectKey;
};
std::string toJsonValueStr(const SelectLspServerParams &val);

} // namesapce newlsp

#endif // EXTENDEDPROJECT_H
