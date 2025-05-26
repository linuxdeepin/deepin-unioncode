// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKENDGLOBAL_H
#define BACKENDGLOBAL_H

//! May change output channal later.
#define Log(message) printf("%s", message);

static const int rootVariablesReference = 0;
static const int registersReference = 1;
static const int childVariablesReferenceBegin = 2;

#endif // BACKENDGLOBAL_H
