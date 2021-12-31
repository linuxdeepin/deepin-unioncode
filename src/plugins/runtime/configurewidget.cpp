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
#include "configurewidget.h"
#include "environmentwidget.h"
#include "common/widget/collapsewidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>

const QString runtimeEnvTitle {"Runtime Environment"};
const QString buildEnvTitle {"build Environment"};

class ConfigureWidgetPrivate
{
    friend class ConfigureWidget;
    QWidget *centrelWidget = nullptr;
    QVBoxLayout *layout = nullptr;
};

ConfigureWidget::ConfigureWidget(QWidget *parent)
    : QScrollArea(parent)
    , d(new ConfigureWidgetPrivate())
{
    setWidgetResizable(true);
    d->centrelWidget = new QWidget();
    d->layout = new QVBoxLayout(d->centrelWidget);
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

void ConfigureWidget::resizeEvent(QResizeEvent *event)
{
    d->centrelWidget->resize(d->centrelWidget->width(), event->size().width());
    QScrollArea::resizeEvent(event);
}
