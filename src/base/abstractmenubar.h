#ifndef ABSTRACTMENUBAR_H
#define ABSTRACTMENUBAR_H

class AbstractMenu;
class AbstractAction;
class AbstractMenuBarPrivate;
class AbstractMenuBar
{
    AbstractMenuBarPrivate *const d;
    friend class WindowKeeper;
    void* qMenuBar();
public:
    explicit AbstractMenuBar(void *qMenuBar);
    virtual ~AbstractMenuBar();
};

#endif // ABSTRACTMENUBAR_H
