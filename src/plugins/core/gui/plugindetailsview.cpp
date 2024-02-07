// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "plugindetailsview.h"

#include <DWidget>
#include <DLabel>
#include <DTextEdit>
#include <DListWidget>

#include <QGridLayout>
#include <QDesktopServices>
#include <QWebEngineView>

DWIDGET_USE_NAMESPACE

DetailsView::DetailsView(QWidget *parent)
    : DWidget(parent)
{
    setupUi();
}

void DetailsView::update(const dpf::PluginMetaObjectPointer &metaInfo)
{
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
}

void DetailsView::setupUi()
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    auto detailLayout = new QHBoxLayout(this);

    metaInfoLabel = new DLabel(this);
    metaInfoLabel->setText(MetaInfo().toHtml());
    metaInfoLabel->setOpenExternalLinks(true);
    metaInfoLabel->setWordWrap(true);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon::fromTheme("plugins-navigation").pixmap(QSize(128, 128)));

    auto webViewLayout = new QHBoxLayout(this);
    QWebEngineView *webView = new QWebEngineView(this);
    // TODO(mozart): load from local.
//    webView->load(QUrl::fromLocalFile("url"));
    webView->load(QUrl("https://ecology.chinauos.com/adaptidentification/doc_new/#document2?dirid=656d40a9bd766615b0b02e5e"));
    webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    webViewLayout->addWidget(webView);

    detailLayout->addWidget(iconLabel);
    detailLayout->addWidget(metaInfoLabel, 1);
    detailLayout->setContentsMargins(64, 10, 0, 0);
    mainLayout->addLayout(detailLayout);
    mainLayout->addLayout(webViewLayout);
}
