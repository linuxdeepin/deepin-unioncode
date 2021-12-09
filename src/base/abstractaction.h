#ifndef ABSTRACTNAVACTION_H
#define ABSTRACTNAVACTION_H

#include <functional>
extern const std::string NAVACTION_RECENT;
extern const std::string NAVACTION_EDIT;
extern const std::string NAVACTION_DEBUG;
extern const std::string NAVACTION_RUNTIME;
class AbstractActionPrivate;
class AbstractAction
{
    AbstractActionPrivate *const d;
    friend class WindowKeeper;
    void *qAction();
public:
    explicit AbstractAction(void *qAction);
    virtual ~AbstractAction();
};

#endif // ABSTRACTNAVACTION_H
