/*
 * Copyright (C) 2020 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             huangyu<huangyub@uniontech.com>
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
#include "perfrecorddisplay.h"

#include <QCheckBox>
#include <QVBoxLayout>

class PerfRecordDisplayPrivate
{
    friend class PerfRecordDisplay;
    QCheckBox *showWebBrowse{nullptr};
    QVBoxLayout *vLayout{nullptr};
};

PerfRecordDisplay::PerfRecordDisplay(QWidget *parent, Qt::WindowFlags f)
    : QWidget (parent, f)
    , d (new PerfRecordDisplayPrivate)
{
    d->showWebBrowse = new QCheckBox(QCheckBox::tr("use WebBrowser show flame-Graph"));

    d->vLayout = new QVBoxLayout();
    d->vLayout->addWidget(d->showWebBrowse);
    setLayout(d->vLayout);

    QObject::connect(d->showWebBrowse, &QCheckBox::toggled,
                     this, &PerfRecordDisplay::showWebBrowserGP);
    d->showWebBrowse->setChecked(false);
}

PerfRecordDisplay::~PerfRecordDisplay()
{
    if (d)
        delete d;
}
