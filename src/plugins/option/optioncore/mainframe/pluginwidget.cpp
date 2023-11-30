// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginwidget.h"
#include "framework/lifecycle/pluginview.h"
#include "framework/lifecycle/lifecycle.h"

#include <DFrame>
#include <DLabel>
#include <DIcon>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
class PluginWidgetPrivate
{
    friend class PluginWidget;
    bool isRestartRequired = false;
    QVBoxLayout *vLayout = nullptr;
    DLabel *iconlabel = nullptr;
    DLabel *restartRequired = nullptr;
    dpf::PluginView *pluginView = nullptr;
};


PluginWidget::PluginWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new PluginWidgetPrivate)
{
    setFixedHeight(492);

    d->pluginView = new dpf::PluginView(this);
    d->vLayout = new QVBoxLayout(this);
    d->vLayout->addWidget(d->pluginView);
    setLayout(d->vLayout);

    QHBoxLayout *hlayout = new QHBoxLayout(this);

    d->restartRequired = new DLabel(tr("Restart takes effect"));
    QIcon icon = QIcon::fromTheme("options_alert");
    d->iconlabel = new DLabel(this);
    d->iconlabel->setPixmap(icon.pixmap(icon.actualSize(QSize(20, 20))));

    hlayout->addWidget(d->iconlabel);
    hlayout->addWidget(d->restartRequired);
    hlayout->setAlignment(Qt::AlignLeft);
    hlayout->setContentsMargins(2, 0, 0, 0);

    if(!d->isRestartRequired){
        d->restartRequired->setVisible(false);
        d->iconlabel->setVisible(false);
    }
    d->vLayout->addLayout(hlayout);

    QObject::connect(d->pluginView, &dpf::PluginView::pluginSettingChanged,
                     this, &PluginWidget::updateRestartRequired);
}

PluginWidget::~PluginWidget()
{

}

void PluginWidget::saveConfig()
{
    dpf::LifeCycle::getPluginManagerInstance()->writeSettings();
}

void PluginWidget::updateRestartRequired()
{
    d->isRestartRequired = true;
    d->restartRequired->setVisible(true);
    d->iconlabel->setVisible(true);
}
