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
#ifndef TOOLCHAIN_H
#define TOOLCHAIN_H
#include <QString>

namespace toolchains {
extern const QString K_VERSION;
extern const QString K_HOST_OS;
extern const QString K_HOST_ARCH;
extern const QString K_HOST_KERNEL;
extern const QString K_TOOLCHAINS;
extern const QString K_TOOLCHAIN_NAME;
extern const QString K_TOOLCHAIN_ABI;
extern const QString K_TOOLCHAIN_PREFIX;
extern const QString K_TOOLCHAIN_PATH;
extern const QString K_TOOLCHAIN_C_COMPILER;
extern const QString K_TOOLCHAIN_CXX_COMPILER;
extern const QString K_TOOLCHAIN_DEBUGGER;
extern const QString K_TOOLCHAINFILE;

bool generatGlobalFile();

} // namespace toolchains

#endif
