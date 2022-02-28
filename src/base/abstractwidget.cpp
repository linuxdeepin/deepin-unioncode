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

    d->qWidget = (QWidget*)qWidget;
    qInfo() << "d->qWidget->objectName" << d->qWidget;
    QObject::connect(d->qWidget, &QWidget::destroyed,
                     d->qWidget, [this](QObject *obj){
        qApp->processEvents(QEventLoop::ProcessEventsFlag::WaitForMoreEvents);
        if (obj == d->qWidget) {
            qInfo() << "delete obj" << obj;
            delete this;
        }
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
