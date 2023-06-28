// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIRTUALCONSOLE_H
#define VIRTUALCONSOLE_H

#include <qtermwidget.h>

class VirtualConsole : public QTermWidget
{
    Q_OBJECT
public:
    explicit VirtualConsole(QWidget *parent = nullptr);
    virtual ~VirtualConsole();
};

#endif // VIRTUALCONSOLE_H
