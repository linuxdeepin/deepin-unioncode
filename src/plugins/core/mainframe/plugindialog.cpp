/*
 * Copyright (C) 2023 Uniontech Software Technology Co., Ltd.
 *
 * Author:     hongjinchuan<hongjinchuan@uniontech.com>
 *
 * Maintainer: hongjinchuan<hongjinchuan@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    resize(650, 450);
    setWindowTitle(tr("Installed Plugins"));

    auto vLayout = new QVBoxLayout(this);
    vLayout->addWidget(view);

    detailsButton = new QPushButton(tr("Details"), this);
    closeButton = new QPushButton(tr("Close"), this);
    detailsButton->setEnabled(false);
    closeButton->setEnabled(true);
    closeButton->setDefault(true);

    restratRequired = new QLabel(tr(" Restart required."), this);
    if (!isRestartRequired)
        restratRequired->setVisible(false);

    auto hLayout = new QHBoxLayout;
    hLayout->addWidget(detailsButton);
    hLayout->addSpacing(10);
    hLayout->addWidget(restratRequired);
    hLayout->addStretch(5);
    hLayout->addWidget(closeButton);
    vLayout->addLayout(hLayout);

    QObject::connect(view, &dpf::PluginView::currentPluginChanged,
                     this, &PluginDialog::updateButtons);
    QObject::connect(view, &dpf::PluginView::currentPluginActived,
                     this, &PluginDialog::openDetails);
    QObject::connect(view, &dpf::PluginView::pluginSettingChanged,
                     this, &PluginDialog::updateRestartRequired);
    QObject::connect(detailsButton, &QPushButton::clicked,
                     [this] { openDetails(); });
    QObject::connect(closeButton, &QPushButton::clicked,
                     [this] { closeDialog() ;});

    updateButtons();
}

void PluginDialog::updateRestartRequired()
{
    isRestartRequired = true;
    restratRequired->setVisible(true);
}

void PluginDialog::updateButtons()
{
    dpf::PluginMetaObjectPointer selectedPlugin = view->currentPlugin();
    if (selectedPlugin) {
        detailsButton->setEnabled(true);
    } else {
        detailsButton->setEnabled(false);
    }
}

void PluginDialog::openDetails()
{
    dpf::PluginMetaObjectPointer plugin = view->currentPlugin();
    if (!plugin)
        return;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Plugin Details of %1").arg(plugin->name()));
    auto vLayout = new QVBoxLayout;
    dialog.setLayout(vLayout);
    auto detailView = new DetailsView(&dialog);
    vLayout->addWidget(detailView);
    detailView->update(plugin);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, &dialog);
    vLayout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.exec();
}

void PluginDialog::closeDialog()
{
    dpf::LifeCycle::getPluginManagerInstance()->writeSettings();
    accept();
}

DetailsView::DetailsView(QWidget *parent)
    : QWidget(parent)
{
    this->resize(850, 500);
    gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(2, 2, 2, 2);

    label_1 = new QLabel(tr("Name:"), this);
    label_1->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_1->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    name = new QLabel(this);
    name->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_1, 0, 0, 1, 1);
    gridLayout->addWidget(name, 0, 1, 1, 1);

    label_2 = new QLabel(tr("Version:"), this);
    label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_2->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    version = new QLabel(this);
    version->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_2, 1, 0, 1, 1);
    gridLayout->addWidget(version, 1, 1, 1, 1);

    label_3 = new QLabel(tr("Compatibility version:"), this);
    label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_3->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    compatVersion = new QLabel(this);
    compatVersion->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_3, 2, 0, 1, 1);
    gridLayout->addWidget(compatVersion, 2, 1, 1, 1);

    label_4 = new QLabel(tr("Vendor:"), this);
    label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_4->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vendor = new QLabel(this);
    vendor->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_4, 3, 0, 1, 1);
    gridLayout->addWidget(vendor, 3, 1, 1, 1);

    label_5 = new QLabel(tr("Copyright:"), this);
    label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_5->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    copyright = new QLabel(this);
    copyright->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_5, 4, 0, 1, 1);
    gridLayout->addWidget(copyright, 4, 1, 1, 1);

    label_6 = new QLabel(tr("URL:"), this);
    label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_6->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    urlLink = new QLabel(this);
    urlLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    gridLayout->addWidget(label_6, 5, 0, 1, 1);
    gridLayout->addWidget(urlLink, 5, 1, 1, 1);

    vboxLayout_1 = new QVBoxLayout();
    label_7 = new QLabel(tr("Description:"), this);
    label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_7->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vboxLayout_1->addWidget(label_7);
    spacerItem_1 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vboxLayout_1->addItem(spacerItem_1);
    description = new QTextEdit(this);
    description->setReadOnly(true);
    gridLayout->addLayout(vboxLayout_1, 6, 0, 1, 1);
    gridLayout->addWidget(description, 6, 1, 1, 1);

    vboxLayout_2 = new QVBoxLayout();
    label_8 = new QLabel(tr("License:"), this);
    label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_8->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vboxLayout_2->addWidget(label_8);
    spacerItem_2 = new QSpacerItem(17, 13, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vboxLayout_2->addItem(spacerItem_2);
    license = new QTextEdit(this);
    license->setReadOnly(true);
    gridLayout->addLayout(vboxLayout_2, 7, 0, 1, 1);
    gridLayout->addWidget(license, 7, 1, 1, 1);

    vboxLayout_3 = new QVBoxLayout();
    label_9 = new QLabel(tr("Dependencies:"), this);
    label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    label_9->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse);
    vboxLayout_3->addWidget(label_9);
    spacerItem_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vboxLayout_3->addItem(spacerItem_3);
    dependencies = new QListWidget(this);
    gridLayout->addLayout(vboxLayout_3, 8, 0, 1, 1);
    gridLayout->addWidget(dependencies, 8, 1, 1, 1);
}

void DetailsView::update(dpf::PluginMetaObjectPointer plugin)
{
    name->setText(plugin->name());
    version->setText(plugin->version());
    compatVersion->setText(plugin->compatVersion());
    vendor->setText(plugin->vendor());
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
    for (dpf::PluginDepend depend : plugin->depends()) {
        dependsList << depend.toString();
    }
    dependencies->addItems(dependsList);

    connect(urlLink, &QLabel::linkActivated, [=](){
        QDesktopServices::openUrl(QUrl(plugin->urlLink()));
    });
}
