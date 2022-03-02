#ifndef ABSTRACTWIDGET_H
#define ABSTRACTWIDGET_H

class AbstractWidgetPrivate;
class AbstractWidget
{
    AbstractWidgetPrivate *const d;
public:
    AbstractWidget(void *qWidget);
    virtual ~AbstractWidget();
    void* qWidget();
};

#endif // ABSTRACTWIDGET_H
