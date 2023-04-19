/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
#include "configpropertywidget.h"
#include "common/common.h"
#include "configureprojpane.h"

#include <QListWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QStyleFactory>
#include <QStackedWidget>

class ConfigPropertyWidgetPrivate
{
    friend class ConfigPropertyWidget;
    QStackedWidget *stackedWidget{nullptr};
    ConfigureWidget *configureProjWidget{nullptr};
    ConfigureProjPane *configureProjPane{nullptr};
};

ConfigPropertyWidget::ConfigPropertyWidget(const QString &language,
                                           const QString &workspace,
                                           QDialog *parent)
    : QDialog (parent)
    , d(new ConfigPropertyWidgetPrivate())
{
    setWindowTitle(tr("Config"));
    // Initialize stackedWidget.
    d->stackedWidget = new QStackedWidget(this);

    // Initialize configure project widget.
    d->configureProjWidget = new ConfigureWidget(d->stackedWidget);
    d->configureProjPane = new ConfigureProjPane(language, workspace, d->configureProjWidget);
    QObject::connect(d->configureProjPane, &ConfigureProjPane::configureDone, [this](const dpfservice::ProjectInfo &info){
        closeWidget();
    });
    d->configureProjWidget->addWidget(d->configureProjPane);

    d->stackedWidget->addWidget(d->configureProjWidget);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(d->stackedWidget);
    setLayout(layout);
    close();
}

ConfigPropertyWidget::~ConfigPropertyWidget()
{
    if (d)
        delete d;
}

void ConfigPropertyWidget::closeWidget()
{
    close();
}
