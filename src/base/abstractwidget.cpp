// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractwidget.h"

#include <DWidget>

#include <QDebug>
#include <QApplication>
#include <QScopedPointer> 

DWIDGET_USE_NAMESPACE

//Private class to encapsulate implementation detailsclass AbstractWidgetPrivate {
public:
    QScopedPointer<DWidget> qWidget; // Use QScopedPointer for memory management
};

AbstractWidget::AbstractWidget(DWidget* qWidget)
    : d(new AbstractWidgetPrivate)
{
    Q_ASSERT(qWidget);
    d->qWidget.reset(qWidget);  // Tomar posesión del widget usando QScopedPointer

    connect(d->qWidget.data(), &QObject::destroyed, this, &QObject::deleteLater); 
}

// Destructorthanks to QScopedPointer ;3
AbstractWidget::~AbstractWidget() = default;

DWidget* AbstractWidget::qWidget() const 
{
    return d->qWidget.data(); // Devuelve un puntero al widget
}

