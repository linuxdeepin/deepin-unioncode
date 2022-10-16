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
#ifndef ENVSEACHER_H
#define ENVSEACHER_H

#include "common/common.h"

#include <QVariantHash>
#include <QRegularExpression>

#include <optional>

inline static QString K_Version{"Version"};
inline static QString K_BinsPath{"BinsPath"};
inline static QString K_PkgsPath{"PkgsPath"};
inline static QString K_UserEnv{"UserEnv"};
inline static QString K_GlobalEnv{"GlobalEnv"};

inline static QString RK_Major{"Major"};
inline static QString RK_Minor{"Minor"};
inline static QString RK_Revision{"Revision"};
inline static QString RK_Build{"Build"};

struct Version {
    std::optional<int> major;
    std::optional<int> minor;
    std::optional<int> revision;
    std::optional<int> build;
};
Q_DECLARE_METATYPE(Version)

struct Env {
    std::optional<QString> binsPath;
    std::optional<QString> pkgsPath;
};
Q_DECLARE_METATYPE(Env)

struct UserEnv : Env {};
Q_DECLARE_METATYPE(UserEnv)

struct GlobalEnv : Env {};
Q_DECLARE_METATYPE(GlobalEnv)

struct EnvSeacher {
    static QVariantHash python3();
    static QVariantHash cxx();
    static QVariantHash java();
};

#endif // ENVSEACHER_H
