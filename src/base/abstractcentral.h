#ifndef ABSTRACTCENTRAL_H
#define ABSTRACTCENTRAL_H

class AbstractCentralPrivate;
class AbstractCentral
{
    AbstractCentralPrivate *const d;
public:
    AbstractCentral(void *qwidget);
    virtual ~AbstractCentral();
    void *qWidget();
};

#endif // ABSTRACTCENTRAL_H
