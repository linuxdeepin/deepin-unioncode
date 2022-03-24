#include "autohidedockwidget.h"

#include <QEvent>

AutoHideDockWidget::AutoHideDockWidget(const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget (title, parent, flags)
{

}

AutoHideDockWidget::AutoHideDockWidget(QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget (parent, flags)
{

}

QWidget *AutoHideDockWidget::widget() const
{
    return QDockWidget::widget();
}

void AutoHideDockWidget::setWidget(QWidget *widget)
{
    if (widget) {
        widget->installEventFilter(this);
    }
    return QDockWidget::setWidget(widget);
}

bool AutoHideDockWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == widget()) {
        switch (e->type()){
        case QEvent::HideToParent:
            QDockWidget::hide();
            return true;
        case QEvent::ShowToParent:
            QDockWidget::show();
            return true;
        case QEvent::Destroy:
            QDockWidget::hide();
            return true;
        default:
            return false;
        }
    }
    return false;
}
