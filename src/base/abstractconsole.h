// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTCONSOLE_H
#define ABSTRACTCONSOLE_H

class AbstractConsolePrivate;
class AbstractConsole
{
    AbstractConsolePrivate *const d;
public:
    AbstractConsole(void *qWidget);
    virtual ~AbstractConsole();
    void* qWidget();
};

#endif // ABSTRACTCONSOLE_H
