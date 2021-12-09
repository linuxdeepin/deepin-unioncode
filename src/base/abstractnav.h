#ifndef ABSTRACTNAV_H
#define ABSTRACTNAV_H

class AbstractAction;
class AbstractNavPrivate;
class AbstractNav
{
    AbstractNavPrivate *const d;
    friend class WindowKeeper;
    void *qToolbar();
public:
    explicit AbstractNav(void *qToolBar);
    virtual ~AbstractNav();
};

#endif // ABSTRACTNAV_H
