/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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

#include "environmentmanagerwidget.h"
#include "shortcutsettingwidget.h"
#include <QtWidgets/QVBoxLayout>
#include <QTabWidget>

class EnvironmentManagerWidgetPrivate
{
    EnvironmentManagerWidgetPrivate();
    ShortcutSettingWidget* shortcutSettingWidget;

    friend class EnvironmentManagerWidget;
};

EnvironmentManagerWidgetPrivate::EnvironmentManagerWidgetPrivate()
    : shortcutSettingWidget(nullptr)
{

}

EnvironmentManagerWidget::EnvironmentManagerWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new EnvironmentManagerWidgetPrivate())
{
    setupUi();
}

EnvironmentManagerWidget::~EnvironmentManagerWidget()
{

}

void EnvironmentManagerWidget::setupUi()
{
    setWindowTitle(tr("Environment"));

    auto tabWidget = new QTabWidget();
    d->shortcutSettingWidget = new ShortcutSettingWidget();

    tabWidget->addTab(d->shortcutSettingWidget, QString("Commands"));
    tabWidget->addTab(new QWidget(), QString("Others"));
    tabWidget->setCurrentIndex(0);

    auto vLayout = new QVBoxLayout();
    vLayout->addWidget(tabWidget);
    setLayout(vLayout);
}

void EnvironmentManagerWidget::saveConfig()
{
    d->shortcutSettingWidget->saveConfig();
}

