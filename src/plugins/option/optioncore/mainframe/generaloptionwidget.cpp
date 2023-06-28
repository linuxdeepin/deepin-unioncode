// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
