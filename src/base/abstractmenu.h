#ifndef ABSTRACTMENU_H
#define ABSTRACTMENU_H

#include "abstractaction.h"

class AbstractMenuPrivate;
class AbstractMenu
{
    AbstractMenuPrivate *const d;
public:
    explicit AbstractMenu(void *qMenu);
    virtual ~AbstractMenu();
    void *qMenu();
};

#endif // ABSTRACTMENU_H
