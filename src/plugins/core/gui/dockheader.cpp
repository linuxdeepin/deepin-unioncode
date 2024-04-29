// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockheader.h"

#include <DStyle>

#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>

DWIDGET_USE_NAMESPACE

class DockHeaderPrivate{
    friend class DockHeader;

    QHBoxLayout *mainLayout { nullptr };
    QLabel *headerName { nullptr };
};

DockHeader::DockHeader(QWidget *parent)
    : DWidget(parent), d(new DockHeaderPrivate)
{
    setAutoFillBackground(true);
    setBackgroundRole(DPalette::Base);

    d->headerName = new QLabel(this);

    d->mainLayout = new QHBoxLayout(this);
    d->mainLayout->setContentsMargins(10, 0, 10, 0);
    d->mainLayout->setAlignment(Qt::AlignRight);
    d->mainLayout->addWidget(d->headerName, Qt::AlignLeft);
}

DockHeader::~DockHeader()
{
    if (d)
        delete d;
}

void DockHeader::addToolButton(DToolButton *btn)
{
    btn->setFixedSize(20, 20);
    d->mainLayout->insertWidget(1, btn);
}

void DockHeader::setHeaderName(const QString &headerName)
{
    d->headerName->setText(headerName);
}
