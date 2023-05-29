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
    enum_exp unionparser = "unionparser";
};
namespace native {
QString path(const Category::type_value &category = "");
bool installed();
} // native
} namespace pkg = package;

}namespace env = environment;


#endif // ENVIRONMENT_H
