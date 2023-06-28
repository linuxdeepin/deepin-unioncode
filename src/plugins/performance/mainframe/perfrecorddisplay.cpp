// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
