#ifndef ABSTRACTMENUBAR_H
#define ABSTRACTMENUBAR_H

class AbstractMenu;
class AbstractAction;
class AbstractMenuBarPrivate;
class AbstractMenuBar
{
    AbstractMenuBarPrivate *const d;
public:
    explicit AbstractMenuBar(void *qMenuBar);
    virtual ~AbstractMenuBar();
    void* qMenuBar();
};

#endif // ABSTRACTMENUBAR_H
