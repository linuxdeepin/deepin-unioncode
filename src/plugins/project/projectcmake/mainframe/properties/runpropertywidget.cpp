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
#include "runpropertywidget.h"
#include "environmentwidget.h"
#include "common/common.h"
#include "runconfigpane.h"

#include <QListWidget>
#include <QSplitter>
#include <QComboBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QStyleFactory>
#include <QStackedWidget>

static RunPropertyWidget *ins{nullptr};

class RunPropertyWidgetPrivate
{
    friend class RunPropertyWidget;
    ConfigureWidget *runCfgWidget{nullptr};
    QStackedWidget *stackedWidget{nullptr};
};

RunPropertyWidget::RunPropertyWidget(QWidget *parent)
    : QSplitter (parent)
    , d(new RunPropertyWidgetPrivate())
{
    // Initialize stackedWidget.
    d->stackedWidget = new QStackedWidget(this);

    // Initialize run config widget.
    d->runCfgWidget = new ConfigureWidget(d->stackedWidget);
    d->runCfgWidget->addWidget(new RunConfigPane(d->runCfgWidget));
    d->runCfgWidget->addWidget(new EnvironmentWidget(d->runCfgWidget));


    d->stackedWidget->addWidget(d->runCfgWidget);


    addWidget(d->stackedWidget);

    setStretchFactor(0, 1);
    setStretchFactor(1, 2);
    setChildrenCollapsible(false);
}

RunPropertyWidget::~RunPropertyWidget()
{
    if (d)
        delete d;
}

