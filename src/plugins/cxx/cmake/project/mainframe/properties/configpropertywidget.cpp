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
    ConfigureWidget *configureProjWidget{nullptr};
    ConfigureProjPane *configureProjPane{nullptr};
};

ConfigPropertyWidget::ConfigPropertyWidget(const QString &language,
                                           const QString &workspace,
                                           QDialog *parent)
    : DDialog (parent)
    , d(new ConfigPropertyWidgetPrivate())
{
    setWindowTitle(tr("Config"));
    setIcon(QIcon::fromTheme("ide"));

    auto mainFrame = new DWidget(this);
    addContent(mainFrame);

    // Initialize configure project widget.
    d->configureProjPane = new ConfigureProjPane(language, workspace, mainFrame);
    QObject::connect(d->configureProjPane, &ConfigureProjPane::configureDone, [this](const dpfservice::ProjectInfo &info) {
        closeWidget();
    });

    QVBoxLayout *layout = new QVBoxLayout(mainFrame);
    layout->addWidget(d->configureProjPane);
    mainFrame->setLayout(layout);

    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("Configure", "button"));
    addButton(buttonTexts[0], false);
    addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    setDefaultButton(1);

    connect(getButton(1),&QAbstractButton::clicked,this,[=](){
        d->configureProjPane->slotConfigure();
    });

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
