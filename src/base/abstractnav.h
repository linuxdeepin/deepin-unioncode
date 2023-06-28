// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTNAV_H
#define ABSTRACTNAV_H

class AbstractAction;
class AbstractNavPrivate;
class AbstractNav
{
    AbstractNavPrivate *const d;
public:
    explicit AbstractNav(void *qToolBar);
    virtual ~AbstractNav();
    void *qToolbar();
};

#endif // ABSTRACTNAV_H
