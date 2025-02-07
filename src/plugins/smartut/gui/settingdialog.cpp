// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingdialog.h"
#include "manager/smartutmanager.h"
#include "utils/utils.h"

#include <DLabel>
#include <DFrame>

#include <QGridLayout>
#include <QFileInfo>
#include <QFileDialog>
#include <QStandardPaths>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

SettingDialog::SettingDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
    initConnection();
}

QStringList SettingDialog::selectedFileList() const
{
    return resourceWidget->selectedFileList();
}

QString SettingDialog::selectedProject() const
{
    return resourceWidget->selectedProject();
}

QString SettingDialog::targetLocation() const
{
    return resourceWidget->targetLocation();
}

void SettingDialog::showEvent(QShowEvent *e)
{
    generalWidget->updateSettings();
    promptWidget->updateSettings();
    resourceWidget->updateSettings();
    DDialog::showEvent(e);
}

void SettingDialog::initUI()
{
    setFixedSize(550, 618);
    setIcon(QIcon::fromTheme("ide"));
    setOnButtonClickedClose(false);

    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(contentWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    btnBox = new DButtonBox(this);
    DButtonBoxButton *generalBtn = new DButtonBoxButton(tr("General"), this);
    DButtonBoxButton *promptBtn = new DButtonBoxButton(tr("Prompt"), this);
    DButtonBoxButton *srcBtn = new DButtonBoxButton(tr("Resource"), this);
    btnBox->setButtonList({ generalBtn, promptBtn, srcBtn }, true);
    btnBox->setId(generalBtn, 0);
    btnBox->setId(promptBtn, 1);
    btnBox->setId(srcBtn, 2);
    generalBtn->setChecked(true);

    mainWidget = new QStackedWidget(this);
    generalWidget = new GeneralSettingWidget(this);
    promptWidget = new PromptSettingWidget(this);
    resourceWidget = new ResourceSettingWidget(this);
    mainWidget->addWidget(generalWidget);
    mainWidget->addWidget(promptWidget);
    mainWidget->addWidget(resourceWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    cancelBtn = new QPushButton(tr("Cancel", "button"), this);
    cancelBtn->setFixedWidth(165);
    okBtn = new DSuggestButton(tr("OK", "button"), this);
    okBtn->setFixedWidth(165);
    btnLayout->addWidget(cancelBtn, 0, Qt::AlignRight);
    btnLayout->addWidget(new DVerticalLine(this));
    btnLayout->addWidget(okBtn, 0, Qt::AlignLeft);

    layout->addWidget(btnBox, 0, Qt::AlignTop | Qt::AlignHCenter);
    layout->addWidget(mainWidget, 1);
    layout->addLayout(btnLayout);
    addContent(contentWidget);
}

void SettingDialog::initConnection()
{
    connect(btnBox, &DButtonBox::buttonClicked, this, &SettingDialog::handleSwitchWidget);
    connect(this, &SettingDialog::buttonClicked, this, &SettingDialog::handleButtonClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &SettingDialog::reject);
    connect(okBtn, &DSuggestButton::clicked, this, &SettingDialog::handleButtonClicked);
}

void SettingDialog::handleSwitchWidget(QAbstractButton *btn)
{
    btn->setChecked(true);
    auto index = btnBox->id(btn);
    mainWidget->setCurrentIndex(index);
}

void SettingDialog::handleButtonClicked()
{
    if (!generalWidget->apply()) {
        int index = mainWidget->indexOf(generalWidget);
        handleSwitchWidget(btnBox->button(index));
        return;
    }

    if (!resourceWidget->apply()) {
        int index = mainWidget->indexOf(resourceWidget);
        handleSwitchWidget(btnBox->button(index));
        return;
    }

    promptWidget->apply();
    accept();
}
