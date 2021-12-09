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
