// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configurewidget.h"
#include "collapsewidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>

DWIDGET_USE_NAMESPACE

class ConfigureWidgetPrivate
{
    friend class ConfigureWidget;
    DWidget *centrelWidget = nullptr;
    QVBoxLayout *layout = nullptr;
};

ConfigureWidget::ConfigureWidget(QWidget *parent)
    : DScrollArea(parent)
    , d(new ConfigureWidgetPrivate())
{
    setAutoFillBackground(true);
    setWidgetResizable(true);
    d->centrelWidget = new DWidget();
    d->centrelWidget->setObjectName("ConfigureCentrelWidget");
    d->centrelWidget->setAutoFillBackground(true);
    d->layout = new QVBoxLayout(d->centrelWidget);
    d->layout->setContentsMargins(24, 24, 24, 24);
    d->layout->setSpacing(24);
    d->centrelWidget->setLayout(d->layout);
    d->layout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    setWidget(d->centrelWidget);
}

ConfigureWidget::~ConfigureWidget()
{
    if (d)
        delete d;
}

void ConfigureWidget::addCollapseWidget(CollapseWidget *widget)
{
    if (!widget)
        return;
    qInfo() << widget->isChecked();
    if (!widget->isChecked()) {
        widget->setChecked(true);
    }
    widget->setFixedHeight(600);
    d->layout->insertWidget(d->layout->count() - 1, widget);
    qInfo() << widget->size();
}

void ConfigureWidget::addWidget(DWidget *widget)
{
    if (!widget)
        return;

    d->layout->insertWidget(d->layout->count() - 1, widget);
}

void ConfigureWidget::resizeEvent(QResizeEvent *event)
{
    d->centrelWidget->resize(d->centrelWidget->width(), event->size().width());
    DScrollArea::resizeEvent(event);
}
