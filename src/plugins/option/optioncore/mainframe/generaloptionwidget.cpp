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
#include "generaloptionwidget.h"
#include "environmentwidget.h"
#include "shortcutsettingwidget.h"
#include "profilesettingwidget.h"

#include <QHBoxLayout>
#include <QTabWidget>

class GeneralOptionWidgetPrivate {
    QTabWidget* tabWidget = nullptr;

    friend class GeneralOptionWidget;
};

GeneralOptionWidget::GeneralOptionWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new GeneralOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new QTabWidget();
    layout->addWidget(d->tabWidget);

    d->tabWidget->addTab(new EnvironmentWidget(), tr("Environment"));
    d->tabWidget->addTab(new ShortcutSettingWidget(), tr("Commands"));
    d->tabWidget->addTab(new ProfileSettingWidget(), tr("Interface"));
    setLayout(layout);
}

GeneralOptionWidget::~GeneralOptionWidget()
{
    if (d)
        delete d;
}

void GeneralOptionWidget::saveConfig()
{
    PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->currentWidget());
    if (pageWidget) {
        pageWidget->saveConfig();
    }
}

void GeneralOptionWidget::readConfig()
{
    PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->currentWidget());
    if (pageWidget) {
        pageWidget->readConfig();
    }
}
