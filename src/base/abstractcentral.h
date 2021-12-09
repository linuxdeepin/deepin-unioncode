#ifndef ABSTRACTCENTRAL_H
#define ABSTRACTCENTRAL_H

class AbstractCentralPrivate;
class AbstractCentral
{
    AbstractCentralPrivate *const d;
    friend class WindowKeeper;
    void *qWidget();
public:
    AbstractCentral(void *qwidget);
    virtual ~AbstractCentral();
};

#endif // ABSTRACTCENTRAL_H
