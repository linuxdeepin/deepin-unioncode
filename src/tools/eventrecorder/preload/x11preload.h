// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _X11_PRELOAD_H
#define _X11_PRELOAD_H

#ifdef __cplusplus
extern "C" {
#endif

#define SIMPLE_X11_HOOK

#ifndef SIMPLE_X11_HOOK
int start_record_x11(const char* displayName, const char* filter);
void stop_record_x11(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // #ifndef _X11_PRELOAD_H
