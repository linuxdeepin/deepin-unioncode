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
#include "javaoptionwidget.h"
#include "jdkwidget.h"

#include "services/option/optionutils.h"
#include "services/option/optiondatastruct.h"
#include "services/option/optionmanager.h"

#include <QHBoxLayout>
#include <QTabWidget>

class JavaOptionWidgetPrivate {
    QTabWidget* tabWidget = nullptr;

    friend class JavaOptionWidget;
};

JavaOptionWidget::JavaOptionWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new JavaOptionWidgetPrivate())
{
    QHBoxLayout *layout = new QHBoxLayout();
    d->tabWidget = new QTabWidget();
    layout->addWidget(d->tabWidget);

    d->tabWidget->addTab(new JDKWidget(), tr("JDK"));
    QObject::connect(d->tabWidget, &QTabWidget::currentChanged, [this]() {
        readConfig();
    });

    setLayout(layout);
}

JavaOptionWidget::~JavaOptionWidget()
{
    if (d)
        delete d;
}

void JavaOptionWidget::saveConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++)
    {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString parentNode = QString::fromLatin1(option::CATEGORY_JAVA);
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            pageWidget->getUserConfig(map);
            OptionUtils::writeJsonSection(OptionUtils::getJsonFilePath(), parentNode, itemNode, map);

            OptionManager::getInstance()->updateData();
        }
    }
}

void JavaOptionWidget::readConfig()
{
    for (int index = 0; index < d->tabWidget->count(); index++)
    {
        PageWidget *pageWidget = qobject_cast<PageWidget*>(d->tabWidget->widget(index));
        if (pageWidget) {
            QString parentNode = QString::fromLatin1(option::CATEGORY_JAVA);
            QString itemNode = d->tabWidget->tabText(d->tabWidget->currentIndex());
            QMap<QString, QVariant> map;
            OptionUtils::readJsonSection(OptionUtils::getJsonFilePath(), parentNode, itemNode, map);
            pageWidget->setUserConfig(map);
        }
    }
}

