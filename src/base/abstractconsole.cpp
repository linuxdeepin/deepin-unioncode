// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractconsole.h"

#include <DWidget>

#include <QDebug>

DWIDGET_USE_NAMESPACE

class AbstractConsolePrivate
{
    friend class AbstractConsole;
    DWidget *qWidget = nullptr;
};

AbstractConsole::AbstractConsole(void *qWidget)
    : d(new AbstractConsolePrivate)
{
    if (!qWidget) {
        qCritical() << "Failed, use QWidget(0x0) to AbstractWidget";
        abort();
    }

    d->qWidget = static_cast<DWidget*>(qWidget);
    d->qWidget->connect(d->qWidget, &DWidget::destroyed,
                        d->qWidget, [this](QObject *obj){
        if (obj == d->qWidget) {
            qInfo() << "AbstractConsole QWidget::destroyed" << obj;
            delete this;
        }
    }, Qt::DirectConnection);
}

AbstractConsole::~AbstractConsole()
{
    if (d) {
        delete d;
    }
}

void *AbstractConsole::qWidget()
{
    return d->qWidget;
}

