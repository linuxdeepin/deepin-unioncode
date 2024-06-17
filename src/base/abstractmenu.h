// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTMENU_H
#define ABSTRACTMENU_H

#include "abstractaction.h"

#include <DMenu>
#include <QList>
#include <memory> // For std::unique_ptr

DWIDGET_USE_NAMESPACE

class AbstractMenu
{
    Q_OBJECT // Add Q_OBJECT macro for signals and slots

public:
    explicit AbstractMenu(DMenu* qMenu);
    virtual ~AbstractMenu() = default; // Virtual destructor (default implementation)

    // Getters 
    DMenu* qMenu() const; 
    QList<AbstractAction*> actionList() const;

    // Add actions
    void addAction(AbstractAction* action);

signals:
    // You can define custom signals here (e.g., void actionTriggered(AbstractAction* action))

private:
    class AbstractMenuPrivate; // Forward declare private implementation class
    std::unique_ptr<AbstractMenuPrivate> d; // Use unique_ptr for automatic memory management
};

#endif // ABSTRACTMENU_H

