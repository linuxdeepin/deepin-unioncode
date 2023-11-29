// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "perfrecorddisplay.h"

#include <DCheckBox>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
class PerfRecordDisplayPrivate
{
    friend class PerfRecordDisplay;
    DCheckBox *showWebBrowse{nullptr};
    QVBoxLayout *vLayout{nullptr};
};

PerfRecordDisplay::PerfRecordDisplay(QWidget *parent, Qt::WindowFlags f)
    : QWidget (parent, f)
    , d (new PerfRecordDisplayPrivate)
{
    d->showWebBrowse = new DCheckBox(DCheckBox::tr("use WebBrowser show flame-Graph"));

    d->vLayout = new QVBoxLayout();
    d->vLayout->addWidget(d->showWebBrowse);
    setLayout(d->vLayout);

    QObject::connect(d->showWebBrowse, &DCheckBox::toggled,
                     this, &PerfRecordDisplay::showWebBrowserGP);
    d->showWebBrowse->setChecked(false);
}

PerfRecordDisplay::~PerfRecordDisplay()
{
    if (d)
        delete d;
}
