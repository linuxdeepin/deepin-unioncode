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
#include "environmentwidget.h"
#include <QVBoxLayout>
#include <QCheckBox>

const QString ENABLE_ALL_ENV = EnvironmentWidget::tr("Enable All Environment");

class EnvironmentWidgetPrivate
{
    friend class EnvironmentWidget;
    QVBoxLayout *vLayout = nullptr;
    QTableView *tableView = nullptr;
    QCheckBox *checkBox = nullptr;
};

EnvironmentWidget::EnvironmentWidget(QWidget *parent)
    : QWidget (parent)
    , d(new EnvironmentWidgetPrivate)
{
    setAutoFillBackground(true);

    if (!d->vLayout)
        d->vLayout = new QVBoxLayout();
    this->setLayout(d->vLayout);

    if (!d->tableView)
        d->tableView = new QTableView();

    if (!d->checkBox)
        d->checkBox = new QCheckBox();
    d->checkBox->setText(ENABLE_ALL_ENV);
    d->vLayout->setSpacing(0);
    d->vLayout->setMargin(0);
    d->vLayout->addWidget(d->checkBox);
    d->vLayout->addWidget(d->tableView);
}

EnvironmentWidget::~EnvironmentWidget()
{
    if(d)
        delete d;
}
