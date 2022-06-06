/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
