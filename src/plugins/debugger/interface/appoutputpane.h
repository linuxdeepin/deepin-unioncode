// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPOUTPUTPANE_H
#define APPOUTPUTPANE_H

#include "debuggerglobals.h"
#include "common/widget/outputpane.h"

#include <QPlainTextEdit>

class OutputWindowPrivate;
class AppOutputPane : public OutputPane
{
    Q_OBJECT
public:
    AppOutputPane();
    ~AppOutputPane() override;
};

#endif   // APPOUTPUTPANE_H
