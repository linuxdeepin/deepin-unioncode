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
#include "runtimewidget.h"
#include "configurewidget.h"
#include "environmentwidget.h"
#include "common/common.h"
#include "stepspane.h"
#include "overviewpane.h"
#include "runconfigpane.h"
#include "projectoptionpane.h"
#include "configureprojpane.h"

#include <QListWidget>
#include <QSplitter>
#include <QComboBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QStyleFactory>
#include <QStackedWidget>

static RuntimeWidget *ins{nullptr};

class RuntimeWidgetPrivate
{
    friend class RuntimeWidget;
    ConfigureWidget *buildCfgWidget{nullptr};
    ConfigureWidget *runCfgWidget{nullptr};
    QStackedWidget *stackedWidget{nullptr};
    ConfigureWidget *configureProjWidget{nullptr};
    ConfigureProjPane *configureProjPane{nullptr};
};

RuntimeWidget::RuntimeWidget(QWidget *parent)
    : QSplitter (parent)
    , d(new RuntimeWidgetPrivate())
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

    // Initialize run config widget.
    d->runCfgWidget = new ConfigureWidget(d->stackedWidget);
    d->runCfgWidget->addWidget(new RunConfigPane(d->runCfgWidget));
    d->runCfgWidget->addWidget(new EnvironmentWidget(d->runCfgWidget));

    // Initialize configure project widget.
    d->configureProjWidget = new ConfigureWidget(d->stackedWidget);
    d->configureProjPane = new ConfigureProjPane(d->configureProjWidget);
    connect(d->configureProjPane, &ConfigureProjPane::configureDone, [this](){
        emit configureDone();
        // reset runtime pane.
        showPane(kBuildCfgPane);
    });
    d->configureProjWidget->addWidget(d->configureProjPane);

    d->stackedWidget->addWidget(d->buildCfgWidget);
    d->stackedWidget->addWidget(d->runCfgWidget);
    d->stackedWidget->addWidget(d->configureProjWidget);

    // Bind option pane signal to config panes.
    auto optionPane = new ProjectOptionPane(this);
    connect(optionPane, &ProjectOptionPane::activeRunCfgPane, this, &RuntimeWidget::slotRunCfgPaneActived);
    connect(optionPane, &ProjectOptionPane::activeBuildCfgPane, this, &RuntimeWidget::slotBuildCfgPaneActived);

    // Insert widgets.
    addWidget(optionPane);
    addWidget(d->stackedWidget);

    setStretchFactor(0, 1);
    setStretchFactor(1, 2);
    setChildrenCollapsible(false);

    // expand environment tree widget.
    buldStepPane->setChecked(false);
}

RuntimeWidget::~RuntimeWidget()
{
    if (d)
        delete d;
}

void RuntimeWidget::showPane(RuntimeWidget::PaneType type, QString args)
{
    d->stackedWidget->setCurrentIndex(type);
    if (type == kConfigurePane) {
        d->configureProjPane->setProjectPath(args);
    }
}

void RuntimeWidget::slotBuildCfgPaneActived()
{
    showPane(kBuildCfgPane);
}

void RuntimeWidget::slotRunCfgPaneActived()
{
    showPane(kRunCfgPane);
}
