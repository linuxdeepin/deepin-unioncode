// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "pluginstorewidget.h"
#include "pluginlistview.h"

#include <DSearchEdit>
#include <DLabel>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

PluginStoreWidget::PluginStoreWidget(QWidget *parent)
    : QWidget(parent)
    , pluginListView(new PluginListView(this))
{
    initializeUi();
}

PluginListView *PluginStoreWidget::getPluginListView() const
{
    return pluginListView;
}

void PluginStoreWidget::slotSearchChanged(const QString &searchText)
{
    pluginListView->filter(searchText);
}

void PluginStoreWidget::initializeUi()
{
    inputEdit = new DSearchEdit(this);
    inputEdit->setPlaceHolder(tr("Search Extension"));
    connect(inputEdit, &DSearchEdit::textChanged,
            this, &PluginStoreWidget::slotSearchChanged);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    setLayout(vLayout);
    vLayout->addWidget(inputEdit);
    vLayout->addWidget(pluginListView);
    vLayout->setContentsMargins(0, 5, 0, 5);
}
