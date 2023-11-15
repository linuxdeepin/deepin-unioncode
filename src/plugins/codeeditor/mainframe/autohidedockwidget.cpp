// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "autohidedockwidget.h"

#include <QEvent>

AutoHideDockWidget::AutoHideDockWidget(const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : DDockWidget (title, parent, flags)
{

}

AutoHideDockWidget::AutoHideDockWidget(QWidget *parent, Qt::WindowFlags flags)
    : DDockWidget (parent, flags)
{

}

DWidget *AutoHideDockWidget::widget() const
{
    return QDockWidget::widget();
}

void AutoHideDockWidget::setWidget(DWidget *widget)
{
    if (widget) {
        widget->installEventFilter(this);
    }
    return DDockWidget::setWidget(widget);
}

bool AutoHideDockWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == widget()) {
        switch (e->type()){
        case QEvent::HideToParent:
            DDockWidget::hide();
            return true;
        case QEvent::ShowToParent:
            DDockWidget::show();
            return true;
        case QEvent::Destroy:
            DDockWidget::hide();
            return true;
        default:
            return false;
        }
    }
    return false;
}
