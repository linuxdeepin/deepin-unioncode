// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockheader.h"

#include <DStyle>

#include <QHBoxLayout>
#include <QDebug>

DWIDGET_USE_NAMESPACE

class DockHeaderPrivate{
    friend class DockHeader;

    QHBoxLayout *mainLayout { nullptr };
};

DockHeader::DockHeader(QWidget *parent)
    : DWidget(parent), d(new DockHeaderPrivate)
{
    setAutoFillBackground(true);
    setBackgroundRole(DPalette::Base);

    d->mainLayout = new QHBoxLayout(this);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->setAlignment(Qt::AlignRight);
}

DockHeader::~DockHeader()
{
    if (d)
        delete d;
}

void DockHeader::addToolButton(DToolButton *btn)
{
    btn->setFixedSize(20, 20);
    d->mainLayout->insertWidget(0, btn);
}
