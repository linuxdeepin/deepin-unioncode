// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVERCMDPARSE_H
#define SERVERCMDPARSE_H

#include "common/common_global.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <optional>

namespace newlsp {
extern const QString port;
extern const QString mode;
extern const QString parentPid;
extern const QString stdio;
extern const QString tcp;
extern const QString portDefault;
extern const QString parentPidDefault;
class ServerCmdParsePrivate;
class COMMON_EXPORT ServerCmdParse : QCommandLineParser
{
    ServerCmdParsePrivate *const d;
public:
    ServerCmdParse(const QCoreApplication &app);
    virtual ~ServerCmdParse();
    std::optional<uint> parentPid() const;
    std::optional<std::string> mode() const;
    std::optional<uint> port() const;
    virtual void bindOptions();
};

}
#endif // SERVERCMDPARSE_H
