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
#include <DSuggestButton>
#include <DFrame>

#include <QGridLayout>
#include <QDesktopServices>
#include <QWebEngineView>
#include <QDir>

DWIDGET_USE_NAMESPACE
using namespace dpfservice;

class AutoZoomWebEngineView : public QWebEngineView {
public:
    explicit AutoZoomWebEngineView(QWidget *parent = nullptr)
        : QWebEngineView(parent) {}

protected:
    void resizeEvent(QResizeEvent *event) override {
        QWebEngineView::resizeEvent(event);

        // first resize is full screen
        if (isFirstResize) {
            isFirstResize = false;
            return;
        }

        QSize newSize = event->size();
        qreal zoomFactor = calculateZoomFactor(newSize);
        setZoomFactor(zoomFactor);
    }

    qreal calculateZoomFactor(const QSize &size) {
        if (size.width() > maxWidth)
            maxWidth = size.width();
        qreal zoomFactor = static_cast<qreal>(size.width()) / maxWidth;
        return zoomFactor;
    }
private:
    int maxWidth = 0;
    bool isFirstResize = true;
};

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

    QString dependencies;
    for (auto depend : metaInfo->depends()) {
        dependencies += depend.toString();
        if (depend.name() != metaInfo->depends().back().name()) {
            dependencies += "、";
        }
    }
    QString licenses;
    for (QString &text : metaInfo->license()) {
        licenses += text.simplified();
        if (text != metaInfo->license().back()) {
            licenses += "、";
        }
    }

    name->setText(metaInfo->name());
    version->setText(metaInfo->version());
    category->setText(metaInfo->category());
    vendor->setText(metaInfo->vendor());
    description->setText(tr("Description") + ": " + metaInfo->description());
    dependency->setText(tr("Dependency") + ": " + dependencies);

    bool pluginIsEnabled = pluginMetaInfo->isEnabledBySettings();
    updateLoadBtnDisplay(pluginIsEnabled);

    QString pluginPath = CustomPaths::global(CustomPaths::Plugins);
    QString pluginLogoPath = pluginPath + QDir::separator() + pluginMetaInfo->name() + ".svg";
    QIcon pluginLogo;
    if (QFile::exists(pluginLogoPath)) {
        pluginLogo = QIcon::fromTheme(pluginLogoPath);
    } else {
        pluginLogo = QIcon::fromTheme("default_plugin");
    }

    logoLabel->setPixmap(pluginLogo.pixmap(QSize(96, 96)));

    webView->load(QUrl(metaInfo->urlLink()));
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
    DFrame *detailFrame = new DFrame(this);
    auto detailLayout = new QHBoxLayout(detailFrame);

    auto midLayout = new QVBoxLayout();
    midLayout->setSpacing(0);
    midLayout->setMargin(0);
    initMetaInfoLayout();

    auto operationLayout = new QHBoxLayout();
    operationLayout->setSpacing(0);
    operationLayout->setMargin(0);
    loadBtn = new DSuggestButton(this);
    loadBtn->setFixedSize(86, 36);
    loadBtn->setToolTip(tr("reLaunch when changed!"));
    loadBtn->setChecked(true);
    connect(loadBtn, &DSuggestButton::clicked, this, &DetailsView::changeLoadBtnState);
    operationLayout->addWidget(loadBtn, 0, Qt::AlignLeft);

    auto *cfgBtn = new DPushButton(this);
    cfgBtn->setIcon(QIcon::fromTheme("options_setting"));
    cfgBtn->setFlat(true);
    connect(cfgBtn, &DPushButton::clicked, this, &DetailsView::showCfgWidget);
    operationLayout->addWidget(cfgBtn, 1, Qt::AlignLeft);

    logoLabel = new QLabel(this);
    auto logo = QIcon::fromTheme("default_plugin");
    logoLabel->setPixmap(logo.pixmap(QSize(96, 96)));

    auto webViewLayout = new QHBoxLayout();
    webView = new AutoZoomWebEngineView(this);
    webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    webViewLayout->addWidget(webView);

    midLayout->addLayout(metaInfoLayout);
    midLayout->addSpacing(10);
    midLayout->addLayout(operationLayout);
    detailLayout->addWidget(logoLabel);
    detailLayout->addSpacing(30);
    detailLayout->addLayout(midLayout, 1);
    detailLayout->setContentsMargins(80, 40, 80, 40);
    mainLayout->addWidget(detailFrame);
    mainLayout->addLayout(webViewLayout);
}

void DetailsView::initMetaInfoLayout()
{
    metaInfoLayout = new QVBoxLayout();
    name = new DLabel(this);
    QFont font = name->font();
    font.setBold(true);
    font.setPointSize(20);
    name->setFont(font);

    version = new DLabel(this);
    category = new DLabel(this);
    category->setForegroundRole(DPalette::LightLively);
    description = new DLabel(this);
    vendor = new DLabel(this);
    dependency = new DLabel(this);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setAlignment(Qt::AlignLeft);
    hbox->setSpacing(10);
    hbox->addWidget(name);
    hbox->addWidget(version);
    hbox->addWidget(category);

    metaInfoLayout->addLayout(hbox);
    metaInfoLayout->addWidget(vendor);
    metaInfoLayout->addSpacing(10);
    metaInfoLayout->addWidget(description);
    metaInfoLayout->addWidget(dependency);
}

void DetailsView::updateLoadBtnDisplay(bool isEnabled)
{
    QString loadOperation = isEnabled ? tr("unload") : tr("load");
    loadBtn->setText(loadOperation);
}
