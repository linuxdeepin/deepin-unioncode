// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compileoutputpane.h"
#include "common/common.h"

CompileOutputPane::CompileOutputPane(QWidget *parent)
    : OutputPane(parent)
{
    setAutoFillBackground(true);
    setBackgroundRole(DPalette::Base);
}

// Add more future here when you need.
