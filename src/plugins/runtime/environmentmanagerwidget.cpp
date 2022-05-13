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

EnvironmentManagerWidget::EnvironmentManagerWidget(QWidget *parent)
    : PageWidget(parent)
    , m_widgetShortcutSetting(nullptr)
{
    setupUi();
}

void EnvironmentManagerWidget::setupUi()
{
    setWindowTitle(tr("Environment"));

    auto tabWidget = new QTabWidget();
    m_widgetShortcutSetting = new ShortcutSettingWidget();

    tabWidget->addTab(m_widgetShortcutSetting, QString("Commands"));
    tabWidget->addTab(new QWidget(), QString("Others"));
    tabWidget->setCurrentIndex(0);

    auto vLayout = new QVBoxLayout();
    vLayout->addWidget(tabWidget);
    setLayout(vLayout);
}

void EnvironmentManagerWidget::saveConfig()
{
    m_widgetShortcutSetting->saveConfig();
}

