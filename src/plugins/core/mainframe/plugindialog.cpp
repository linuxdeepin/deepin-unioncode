// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugindialog.h"

#include "framework/lifecycle/lifecycle.h"
#include "framework/lifecycle/pluginmanager.h"
#include "framework/lifecycle/pluginmetaobject.h"
#include "framework/lifecycle/pluginview.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QDesktopServices>

static bool isRestartRequired = false;

PluginDialog::PluginDialog(QWidget *parent)
    : QDialog(parent),
      view(new dpf::PluginView(this))
{
    resize(1000, 600);
    setWindowTitle(tr("Installed Plugins"));

    auto vLayout = new QVBoxLayout(this);
    auto detailLayout = new QHBoxLayout;
    detailLayout->addWidget(view);

    closeButton = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    closeButton->button(QDialogButtonBox::Close)->setText(tr("Close"));
    closeButton->setEnabled(true);

    restratRequired = new QLabel(tr(" Restart required."), this);
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

    vLayout->addLayout(detailLayout);
    vLayout->addLayout(buttonLayout);

    QObject::connect(view, &dpf::PluginView::currentPluginActived, this, &PluginDialog::slotCurrentPluginActived);
    QObject::connect(view, &dpf::PluginView::pluginSettingChanged,
                     this, &PluginDialog::updateRestartRequired);
    QObject::connect(closeButton->button(QDialogButtonBox::Close), &QPushButton::clicked,
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
    : QWidget(parent)
{
    gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(2, 2, 2, 2);

    label_1 = new QLabel(tr("Name:"), this);
    label_1->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    name = new QLabel(this);
    name->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_1, 0, 0, 1, 1);
    gridLayout->addWidget(name, 0, 1, 1, 1);

    label_2 = new QLabel(tr("Version:"), this);
    label_2->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    version = new QLabel(this);
    version->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_2, 1, 0, 1, 1);
    gridLayout->addWidget(version, 1, 1, 1, 1);

    label_3 = new QLabel(tr("Compatibility version:"), this);
    label_3->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    compatVersion = new QLabel(this);
    compatVersion->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_3, 2, 0, 1, 1);
    gridLayout->addWidget(compatVersion, 2, 1, 1, 1);

    label_4 = new QLabel(tr("Vendor:"), this);
    label_4->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vendor = new QLabel(this);
    vendor->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_4, 3, 0, 1, 1);
    gridLayout->addWidget(vendor, 3, 1, 1, 1);

    label_5 = new QLabel(tr("Copyright:"), this);
    label_5->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    copyright = new QLabel(this);
    copyright->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_5, 4, 0, 1, 1);
    gridLayout->addWidget(copyright, 4, 1, 1, 1);

    label_6 = new QLabel(tr("Category:"), this);
    label_6->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    category = new QLabel(this);
    category->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_6, 5, 0, 1, 1);
    gridLayout->addWidget(category, 5, 1, 1, 1);

    label_7 = new QLabel(tr("URL:"), this);
    label_7->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    urlLink = new QLabel(this);
    urlLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_7, 6, 0, 1, 1);
    gridLayout->addWidget(urlLink, 6, 1, 1, 1);

    vboxLayout_1 = new QVBoxLayout();
    label_8 = new QLabel(tr("License:"), this);
    label_8->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vboxLayout_1->addWidget(label_8);
    spacerItem_1 = new QSpacerItem(17, 13, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vboxLayout_1->addItem(spacerItem_1);
    license = new QTextEdit(this);
    license->setReadOnly(true);
    gridLayout->addLayout(vboxLayout_1, 7, 0, 1, 1);
    gridLayout->addWidget(license, 7, 1, 1, 1);

    vboxLayout_2 = new QVBoxLayout();
    label_9 = new QLabel(tr("Description:"), this);
    label_9->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vboxLayout_2->addWidget(label_9);
    spacerItem_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vboxLayout_2->addItem(spacerItem_2);
    description = new QTextEdit(this);
    description->setReadOnly(true);
    gridLayout->addLayout(vboxLayout_2, 8, 0, 1, 1);
    gridLayout->addWidget(description, 8, 1, 1, 1);

    vboxLayout_3 = new QVBoxLayout();
    label_10 = new QLabel(tr("Dependencies:"), this);
    label_10->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vboxLayout_3->addWidget(label_10);
    spacerItem_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vboxLayout_3->addItem(spacerItem_3);
    dependencies = new QListWidget(this);
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

    connect(urlLink, &QLabel::linkActivated, [=](){
        QDesktopServices::openUrl(QUrl(plugin->urlLink()));
    });
}
