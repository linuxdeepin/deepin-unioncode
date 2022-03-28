#ifndef ABSTRACTNAVACTION_H
#define ABSTRACTNAVACTION_H

#include <functional>
class AbstractActionPrivate;
class AbstractAction
{
    AbstractActionPrivate *const d;
public:
    explicit AbstractAction(void *qAction);
    virtual ~AbstractAction();
    void *qAction();
};

#endif // ABSTRACTNAVACTION_H
