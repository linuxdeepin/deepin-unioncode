// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "plugindetailsview.h"
#include "framework/lifecycle/lifecycle.h"
#include "services/option/optionservice.h"
#include "common/util/eventdefinitions.h"

#include <DWidget>
#include <DLabel>
#include <DTextEdit>
#include <DListWidget>
#include <DPushButton>

#include <QGridLayout>
#include <QDesktopServices>
#include <QWebEngineView>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

DetailsView::DetailsView(QWidget *parent)
    : DWidget(parent)
{
    setupUi();
}

DetailsView::~DetailsView()
{
    if (webView) {
        webView->stop();
        // delete webView will cause app cannot exit.
        webView->setParent(nullptr);
    }
}

void DetailsView::update(const dpf::PluginMetaObjectPointer &metaInfo)
{
    pluginMetaInfo = metaInfo;

    if (metaInfo.isNull())
        return;

    MetaInfo updateMetaInfo;
    for (auto depend : metaInfo->depends()) {
        updateMetaInfo.dependency += depend.toString();
        if (depend.name() != metaInfo->depends().back().name()) {
            updateMetaInfo.dependency += "、";
        }
    }
    for (QString &text : metaInfo->license()) {
        updateMetaInfo.license += text.simplified();
        if (text != metaInfo->license().back()) {
            updateMetaInfo.license += "、";
        }
    }
    updateMetaInfo.name = metaInfo->name();
    updateMetaInfo.version = metaInfo->version();
    updateMetaInfo.compatibleVersion = metaInfo->compatVersion();
    updateMetaInfo.vendor = metaInfo->vendor();
    updateMetaInfo.copyright = metaInfo->copyright();
    updateMetaInfo.category = metaInfo->category();
    updateMetaInfo.url = metaInfo->urlLink();
    updateMetaInfo.description = metaInfo->description();

    metaInfoLabel->setText(updateMetaInfo.toHtml());

    bool pluginIsEnabled = pluginMetaInfo->isEnabledBySettings();
    updateLoadBtnDisplay(pluginIsEnabled);

    webView->load(QUrl(updateMetaInfo.url));
}

void DetailsView::changeLoadBtnState()
{
    if (pluginMetaInfo.isNull())
        return;

    bool isEnabled = !pluginMetaInfo->isEnabledBySettings();
    updateLoadBtnDisplay(isEnabled);

    pluginMetaInfo->setEnabledBySettings(isEnabled);
    dpf::LifeCycle::getPluginManagerInstance()->writeSettings();
}

void DetailsView::showCfgWidget()
{
    dpfGetService(OptionService)->showOptionDialog(pluginMetaInfo->name());
    options.showCfgDialg(pluginMetaInfo->name());
}

void DetailsView::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    auto detailLayout = new QHBoxLayout(this);

    auto midLayout = new QVBoxLayout(this);
    midLayout->setSpacing(0);
    midLayout->setMargin(0);
    metaInfoLabel = new DLabel(this);
    metaInfoLabel->setText(MetaInfo().toHtml());
    metaInfoLabel->setOpenExternalLinks(true);
    metaInfoLabel->setWordWrap(true);
    midLayout->addWidget(metaInfoLabel);

    auto operationLayout = new QHBoxLayout(this);
    operationLayout->setSpacing(0);
    operationLayout->setMargin(0);
    loadBtn = new DPushButton(this);
    loadBtn->setToolTip(tr("reLaunch when changed!"));
    connect(loadBtn, &DPushButton::clicked, this, &DetailsView::changeLoadBtnState);
    operationLayout->addWidget(loadBtn, 0, Qt::AlignLeft);

    auto *cfgBtn = new DPushButton(this);
    cfgBtn->setIcon(QIcon::fromTheme("options_setting"));
    cfgBtn->setFlat(true);
    connect(cfgBtn, &DPushButton::clicked, this, &DetailsView::showCfgWidget);
    operationLayout->addWidget(cfgBtn, 1, Qt::AlignLeft);

    QLabel *logoLabel = new QLabel(this);
    logoLabel->setPixmap(QIcon::fromTheme("defaultplugin").pixmap(QSize(128, 128)));

    auto webViewLayout = new QHBoxLayout(this);
    webView = new QWebEngineView();
    webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    webViewLayout->addWidget(webView);

    midLayout->addLayout(operationLayout);
    detailLayout->addWidget(logoLabel);
    detailLayout->addLayout(midLayout, 1);
    detailLayout->setContentsMargins(64, 10, 0, 10);
    mainLayout->addLayout(detailLayout);
    mainLayout->addLayout(webViewLayout);
}

void DetailsView::updateLoadBtnDisplay(bool isEnabled)
{
    QString loadOperation = isEnabled ? tr("unload") : tr("load");
    loadBtn->setText(loadOperation);
}
