// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPILEOUTPUTPANE_H
#define COMPILEOUTPUTPANE_H

#include "common/widget/outputpane.h"

class CompileOutputPane : public OutputPane
{
    Q_OBJECT
public:
    CompileOutputPane(QWidget *parent = nullptr);
};

#endif // COMPILEOUTPUTPANE_H
