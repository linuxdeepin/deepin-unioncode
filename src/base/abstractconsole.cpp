// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractconsole.h"

#include <QWidget>
#include <QDebug>

class AbstractConsolePrivate
{
    friend class AbstractConsole;
    QWidget *qWidget = nullptr;
};

AbstractConsole::AbstractConsole(void *qWidget)
    : d(new AbstractConsolePrivate)
{
    if (!qWidget) {
        qCritical() << "Failed, use QWidget(0x0) to AbstractWidget";
        abort();
    }

    qInfo() << "AbstractConsole Construct from: " << (QWidget*)(qWidget);
    d->qWidget = (QWidget*)qWidget;
    d->qWidget->connect(d->qWidget, &QWidget::destroyed,
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

