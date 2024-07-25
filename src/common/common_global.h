// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMON_GLOBAL_H
#define COMMON_GLOBAL_H


#if defined(COMMON_LIBRARY)
#  define COMMON_EXPORT Q_DECL_EXPORT
#else
#  define COMMON_EXPORT Q_DECL_IMPORT
#endif


#endif // COMMON_GLOBAL_H
