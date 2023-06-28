// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _REPLAY_H
#define _REPLAY_H

int replay(const char* parent_dir, const char* trace_dir, int pid);
int dump(const char* parent_dir, const char* trace_dir, int pid);
int list_pid(const char* parent_dir, const char* trace_dir);

#endif // _REPLAY_H

