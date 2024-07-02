// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GLOBALCONFIGURE_H
#define GLOBALCONFIGURE_H

#include <QString>

class CustomPaths final
{
    CustomPaths() = delete;
    Q_DISABLE_COPY(CustomPaths)
public:
    enum Flags{
        Applition,
        DependLibs,
        Plugins,
        Tools,
        Packages,
        Extensions,
        Resources,
        Configures,
        Scripts,
        Translations,
        Templates
    };
    static QString user(Flags flags);
    static QString global(Flags flags);
    static bool installed();
    static QString endSeparator(const QString &path);
    static QString projectGeneratePath(const QString &path);
    static QString lspRuntimePath(const QString &language);
    static bool checkDir(const QString &path);
    static QString projectCachePath(const QString &projectPath);
};

#endif // GLOBALCONFIGURE_H
