#include "abstractwidget.h"

#include <QWidget>
#include <QDebug>
#include <QApplication>

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

    qInfo() << "AbstractWidget construct from: " << (QWidget*)(qWidget);
    d->qWidget = (QWidget*)qWidget;
    QObject::connect(d->qWidget, &QWidget::destroyed,
                     d->qWidget, [this](QObject *obj){
        if (obj == d->qWidget) {
            qInfo() << "AbstractWidget QWidget::destroyed" << obj;
            delete this;
        }
    }, Qt::DirectConnection);
}

AbstractWidget::~AbstractWidget()
{
    if (d)
        delete d;
}

void *AbstractWidget::qWidget()
{
    return d->qWidget;
}
