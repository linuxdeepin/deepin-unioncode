// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
