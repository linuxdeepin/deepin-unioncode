// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <common/type/menuext.h>
#include <QProcessEnvironment>
#include <optional>

namespace environment {
namespace language {

enum Category{User,System};
enum Kit{Python};

struct Version final
{
    std::optional<int> major;
    std::optional<int> minor;
    std::optional<int> revision;
    std::optional<int> build;
    Version();
    Version(int major);
    Version(const Version &version);
    Version &operator = (const Version &version);
};

struct Program final
{
    std::optional<QString> path;
    Version version;
    std::optional<QString> binsPath;
    std::optional<QString> pkgsPath;
};

Version completion(Category category, Kit kit, const Version &version);
Program search(Category category, Kit kit, const Version& version);
QProcessEnvironment get(Category category, Kit kit, const Version& version);
}namespace lang = language;

namespace package {
enum_def(Category, QString)
{
    enum_exp clangd = "clangd";
    enum_exp jdtls = "jdt-language-server";
};
namespace native {
QString path(const Category::type_value &category = "");
bool installed();
} // native
} namespace pkg = package;

}namespace env = environment;


#endif // ENVIRONMENT_H
