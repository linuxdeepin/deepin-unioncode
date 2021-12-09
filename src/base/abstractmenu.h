#ifndef ABSTRACTMENU_H
#define ABSTRACTMENU_H

#include "abstractaction.h"

extern const std::string MENU_FILE;
extern const std::string MENU_BUILD;
extern const std::string MENU_DEBUG;
extern const std::string MENU_TOOLS;
extern const std::string MENU_HELP;
extern const std::string MENU_CODETREE;

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
