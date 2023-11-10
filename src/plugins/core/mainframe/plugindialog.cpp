// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugindialog.h"

#include "framework/lifecycle/lifecycle.h"
#include "framework/lifecycle/pluginmanager.h"
#include "framework/lifecycle/pluginmetaobject.h"
#include "framework/lifecycle/pluginview.h"

#include <DTitlebar>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopServices>

static bool isRestartRequired = false;

PluginDialog::PluginDialog(QWidget *parent)
    : DAbstractDialog(parent),
      view(new dpf::PluginView(this))
{
    resize(1000, 600);

    DTitlebar *titleBar = new DTitlebar();
    titleBar = new DTitlebar();
    titleBar->setMenuVisible(false);
    titleBar->setTitle(QString(tr("Installed Plugins")));

    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    auto detailLayout = new QHBoxLayout;
    detailLayout->addWidget(view);

    closeButton = new DDialogButtonBox(DDialogButtonBox::Close, Qt::Horizontal, this);
    closeButton->button(DDialogButtonBox::Close)->setText(tr("Close"));
    closeButton->setEnabled(true);

    restratRequired = new DLabel(tr(" Restart required."), this);
    if (!isRestartRequired)
        restratRequired->setVisible(false);

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(restratRequired);
    buttonLayout->addStretch(5);
    buttonLayout->addWidget(closeButton);

    detailView = new DetailsView();
    detailLayout->addWidget(detailView);
    slotCurrentPluginActived();

    vLayout->addWidget(titleBar);
    vLayout->addLayout(detailLayout);
    vLayout->addLayout(buttonLayout);

    QObject::connect(view, &dpf::PluginView::currentPluginActived, this, &PluginDialog::slotCurrentPluginActived);
    QObject::connect(view, &dpf::PluginView::pluginSettingChanged,
                     this, &PluginDialog::updateRestartRequired);
    QObject::connect(closeButton->button(DDialogButtonBox::Close), &DPushButton::clicked,
                     [this] { closeDialog() ;});
}

void PluginDialog::slotCurrentPluginActived()
{
    dpf::PluginMetaObjectPointer plugin = view->currentPlugin();
    detailView->update(plugin);
}

void PluginDialog::updateRestartRequired()
{
    isRestartRequired = true;
    restratRequired->setVisible(true);
}

void PluginDialog::closeDialog()
{
    dpf::LifeCycle::getPluginManagerInstance()->writeSettings();
    accept();
}

DetailsView::DetailsView(QWidget *parent)
    : DWidget(parent)
{
    gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(2, 2, 2, 2);

    nameLabel = new DLabel(tr("Name:"), this);
    nameLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    name = new DLabel(this);
    name->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(nameLabel, 0, 0, 1, 1);
    gridLayout->addWidget(name, 0, 1, 1, 1);

    versionLabel = new DLabel(tr("Version:"), this);
    versionLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    version = new DLabel(this);
    version->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(versionLabel, 1, 0, 1, 1);
    gridLayout->addWidget(version, 1, 1, 1, 1);

    compatVersionLabel = new DLabel(tr("Compatibility version:"), this);
    compatVersionLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    compatVersion = new DLabel(this);
    compatVersion->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(compatVersionLabel, 2, 0, 1, 1);
    gridLayout->addWidget(compatVersion, 2, 1, 1, 1);

    vendorLabel = new DLabel(tr("Vendor:"), this);
    vendorLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vendor = new DLabel(this);
    vendor->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(vendorLabel, 3, 0, 1, 1);
    gridLayout->addWidget(vendor, 3, 1, 1, 1);

    copyrightLabel = new DLabel(tr("Copyright:"), this);
    copyrightLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    copyright = new DLabel(this);
    copyright->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(copyrightLabel, 4, 0, 1, 1);
    gridLayout->addWidget(copyright, 4, 1, 1, 1);

    categoryLabel = new DLabel(tr("Category:"), this);
    categoryLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    category = new DLabel(this);
    category->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(categoryLabel, 5, 0, 1, 1);
    gridLayout->addWidget(category, 5, 1, 1, 1);

    urlLabel = new DLabel(tr("URL:"), this);
    urlLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    urlLink = new DLabel(this);
    urlLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(urlLabel, 6, 0, 1, 1);
    gridLayout->addWidget(urlLink, 6, 1, 1, 1);

    vboxLayout_1 = new QVBoxLayout();
    licenseLabel = new DLabel(tr("License:"), this);
    licenseLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vboxLayout_1->addWidget(licenseLabel);
    spacerItem_1 = new QSpacerItem(17, 13, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vboxLayout_1->addItem(spacerItem_1);
    license = new DTextEdit(this);
    license->setReadOnly(true);
    gridLayout->addLayout(vboxLayout_1, 7, 0, 1, 1);
    gridLayout->addWidget(license, 7, 1, 1, 1);

    vboxLayout_2 = new QVBoxLayout();
    descriptionLabel = new DLabel(tr("Description:"), this);
    descriptionLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vboxLayout_2->addWidget(descriptionLabel);
    spacerItem_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vboxLayout_2->addItem(spacerItem_2);
    description = new DTextEdit(this);
    description->setReadOnly(true);
    gridLayout->addLayout(vboxLayout_2, 8, 0, 1, 1);
    gridLayout->addWidget(description, 8, 1, 1, 1);

    vboxLayout_3 = new QVBoxLayout();
    dependenciesLabel = new DLabel(tr("Dependencies:"), this);
    dependenciesLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vboxLayout_3->addWidget(dependenciesLabel);
    spacerItem_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vboxLayout_3->addItem(spacerItem_3);
    dependencies = new DListWidget(this);
    gridLayout->addLayout(vboxLayout_3, 9, 0, 1, 1);
    gridLayout->addWidget(dependencies, 9, 1, 1, 1);
}

void DetailsView::update(dpf::PluginMetaObjectPointer plugin)
{
    name->setText(plugin->name());
    version->setText(plugin->version());
    compatVersion->setText(plugin->compatVersion());
    vendor->setText(plugin->vendor());
    category->setText(plugin->category());
    const QString link = QString::fromLatin1("<a href=\"%1\">%1</a>").arg(plugin->urlLink());
    urlLink->setText(link);
    copyright->setText(plugin->copyright());
    description->setText(plugin->description());
    QString licenseText;
    for (QString &text : plugin->license()) {
        licenseText += text.simplified();
        licenseText += "\n\n";
    }
    license->setText(licenseText.trimmed());
    QStringList dependsList;
    dependencies->clear();
    for (dpf::PluginDepend depend : plugin->depends()) {
        dependsList << depend.toString();
    }
    dependencies->addItems(dependsList);

    connect(urlLink, &DLabel::linkActivated, [=](){
        QDesktopServices::openUrl(QUrl(plugin->urlLink()));
    });
}
