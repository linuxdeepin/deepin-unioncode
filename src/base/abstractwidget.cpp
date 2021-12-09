#include "abstractwidget.h"

#include <QWidget>
#include <QDebug>

class AbstractWidgetPrivate
{
    friend class AbstractWidget;
    QWidget *qWidget;
};

AbstractWidget::AbstractWidget(void *qWidget)
    : d(new AbstractWidgetPrivate)
{
    if (!qWidget) {
        qCritical() << "Failed, use QWidget(0x0) to AbstractWidget";
        abort();
    }

    d->qWidget = (QWidget*)qWidget;
    QObject::connect(d->qWidget, &QWidget::destroyed,
                     d->qWidget, [this](){
        d->qWidget->dumpObjectInfo();
        delete this;
    }, Qt::UniqueConnection);
}

AbstractWidget::~AbstractWidget()
{
    if (d)
        delete d;
}

void *AbstractWidget::qWidegt()
{
    return d->qWidget;
}
