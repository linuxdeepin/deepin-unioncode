// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBAL_DEFINE_H
#define GLOBAL_DEFINE_H

#include <QVariant>

inline constexpr char CMakeKitName[] { "CMake" };
inline constexpr char OptionID[] { "OptionId" };

// configure
// option
inline constexpr char KitOptionName[] { "Kits" };
inline constexpr char CompilerOptionName[] { "Compilers" };
inline constexpr char DebuggerOptionName[] { "Debuggers" };
inline constexpr char CMakeToolOptionName[] { "CMakeTools" };

// item key
inline constexpr char kCCompiler[] { "ccompiler" };
inline constexpr char kCXXCompiler[] { "cppcompiler" };
inline constexpr char kDebugger[] { "debugger" };
inline constexpr char kCMakeTool[] { "cmake" };
inline constexpr char kCMakeGenerator[] { "cmakeGenerator" };
inline constexpr char kID[] { "id" };

inline constexpr char Name[] { "name" };
inline constexpr char Path[] { "path" };

struct Option
{
    QString name;
    QString path;
    QVariant userData;

    static QString displayName(const QString &name, const QString &path)
    {
        return path.isEmpty()
                ? name
                : QString("%1 (%2)").arg(name, path);
    }

    inline bool operator==(const Option &other) const
    {
        return name == other.name && path == other.path;
    }
};

Q_DECLARE_METATYPE(Option)

#endif   // GLOBAL_DEFINE_H
