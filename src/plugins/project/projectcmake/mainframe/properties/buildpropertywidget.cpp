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
#include "buildpropertywidget.h"
#include "environmentwidget.h"
#include "common/common.h"
#include "stepspane.h"
#include "overviewpane.h"

#include <QListWidget>
#include <QSplitter>
#include <QComboBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QStyleFactory>
#include <QStackedWidget>

static BuildPropertyWidget *ins{nullptr};

class BuildPropertyWidgetPrivate
{
    friend class BuildPropertyWidget;
    ConfigureWidget *buildCfgWidget{nullptr};
    QStackedWidget *stackedWidget{nullptr};
};

BuildPropertyWidget::BuildPropertyWidget(QWidget *parent)
    : QSplitter (parent)
    , d(new BuildPropertyWidgetPrivate())
{
    // Initialize stackedWidget.
    d->stackedWidget = new QStackedWidget(this);

    // Initialize build config widget.
    d->buildCfgWidget = new ConfigureWidget(d->stackedWidget);
    auto overViewPane = new OverviewPane(d->buildCfgWidget);
    auto buldStepPane = new CollapseWidget("Build Steps", new StepsPane(StepsPane::kBuild), d->buildCfgWidget);
    d->buildCfgWidget->addWidget(overViewPane);
    d->buildCfgWidget->addCollapseWidget(buldStepPane);
    d->buildCfgWidget->addCollapseWidget(new CollapseWidget("Clean Steps", new StepsPane(StepsPane::kClean, d->buildCfgWidget)));
    d->buildCfgWidget->addCollapseWidget(new CollapseWidget("Runtime Env", new EnvironmentWidget(d->buildCfgWidget)));

    d->stackedWidget->addWidget(d->buildCfgWidget);

    addWidget(d->stackedWidget);

    setStretchFactor(0, 1);
    setStretchFactor(1, 2);
    setChildrenCollapsible(false);

    // expand environment tree widget.
    buldStepPane->setChecked(false);
}

BuildPropertyWidget::~BuildPropertyWidget()
{
    if (d)
        delete d;
}
