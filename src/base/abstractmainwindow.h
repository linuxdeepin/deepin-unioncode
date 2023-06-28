// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTMAINWINDOW_H
#define ABSTRACTMAINWINDOW_H

#include <string>
#include <list>
class AbstractNav;
class AbstractNavAction;
class AbstractMenuBar;
class AbstractMainWindowPrivate;
namespace WindowContainer {

} // namespace Container

class AbstractMainWindow
{
    AbstractMainWindowPrivate *const d;
public:
    explicit AbstractMainWindow(void *qMainWindow);
    virtual ~AbstractMainWindow();
    void *qMainWindow();
};

#endif // ABSTRACTMAINWINDOW_H
