// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configpropertywidget.h"
#include "common/common.h"
#include "configureprojpane.h"

#include <DStackedWidget>
#include <DListWidget>
#include <DSplitter>
#include <DPushButton>
#include <DFrame>

#include <QStyleFactory>
#include <QVBoxLayout>
#include <QGroupBox>

DWIDGET_USE_NAMESPACE
class ConfigPropertyWidgetPrivate
{
    friend class ConfigPropertyWidget;
    ConfigureProjPane *configureProjPane{ nullptr };
};

ConfigPropertyWidget::ConfigPropertyWidget(const QString &language,
                                           const QString &workspace,
                                           QWidget *parent)
    : DWidget (parent)
    , d(new ConfigPropertyWidgetPrivate())
{
    // Initialize configure project widget.
    d->configureProjPane = new ConfigureProjPane(language, workspace, this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(d->configureProjPane);
}

ConfigPropertyWidget::~ConfigPropertyWidget()
{
    if (d)
        delete d;
}

void ConfigPropertyWidget::accept()
{
    d->configureProjPane->slotConfigure();
}
