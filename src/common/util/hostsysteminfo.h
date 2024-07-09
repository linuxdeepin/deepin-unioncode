// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HOSTSYSTEMINFO_H
#define HOSTSYSTEMINFO_H

#include "common/common_global.h"
#include "util/oshelper.h"

#include <QString>

namespace Utils {

class COMMON_EXPORT HostSystemInfo
{
public:
    static constexpr OsType hostOs()
    {
#if defined(Q_OS_WIN)
        return OsTypeWindows;
#elif defined(Q_OS_LINUX)
        return OsTypeLinux;
#elif defined(Q_OS_UNIX)
        return OsTypeOtherUnix;
#else
        return OsTypeOther;
#endif
    }

    static constexpr bool isWindowsHost() { return hostOs() == OsTypeWindows; }
    static constexpr bool isLinuxHost() { return hostOs() == OsTypeLinux; }
    static constexpr bool isAnyUnixHost()
    {
#ifdef Q_OS_UNIX
        return true;
#else
        return false;
#endif
    }

    static QString withExecutableSuffix(const QString &executable)
    {
        return OsHelper::withExecutableSuffix(hostOs(), executable);
    }

    static QChar pathListSeparator()
    {
        return OsHelper::pathListSeparator(hostOs());
    }
};

} // namespace Utils

#endif // HOSTSYSTEMINFO_H
