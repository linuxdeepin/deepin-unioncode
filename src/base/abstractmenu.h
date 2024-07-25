// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTMENU_H
#define ABSTRACTMENU_H

#include "abstractaction.h"

#include <DMenu>

#include <QList>

DWIDGET_USE_NAMESPACE

class AbstractMenuPrivate;
class AbstractAction;
class AbstractMenu
{
    AbstractMenuPrivate *const d;
public:
    explicit AbstractMenu(DMenu *qMenu);
    virtual ~AbstractMenu();
    DMenu *qMenu();
    void addAction(AbstractAction *action);
    QList<AbstractAction *> actionList();
};

#endif // ABSTRACTMENU_H
