// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mavenwidget.h"
#include "services/option/toolchaindata.h"
#include "common/util/custompaths.h"

#include <DLabel>
#include <DHeaderView>
#include <DLineEdit>
#include <DPushButton>
#include <DComboBox>
#include <DFileDialog>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

DWIDGET_USE_NAMESPACE
class MavenWidgetPrivate
{
    friend class MavenWidget;

    DComboBox *homePathComboBox = nullptr;
    DLineEdit *userSettingEdit = nullptr;
    DLineEdit *localSettingEdit = nullptr;
    QSharedPointer<ToolChainData> toolChainData;
};

MavenWidget::MavenWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new MavenWidgetPrivate())
{
    d->toolChainData.reset(new ToolChainData());
    QString retMsg;
    bool ret = d->toolChainData->readToolChainData(retMsg);
    if (ret) {
        qInfo() << retMsg;
    }

    setupUi();
    updateUi();
}

MavenWidget::~MavenWidget()
{
    if (d) {
        delete d;
    }
}

void MavenWidget::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    DLabel *homePathLabel = new DLabel(tr("Maven path："));
    homePathLabel->setFixedWidth(120);
    d->homePathComboBox = new DComboBox();

    QHBoxLayout *homePathLayout = new QHBoxLayout();
    homePathLayout->addWidget(homePathLabel);
    homePathLayout->addWidget(d->homePathComboBox);

    DLabel *userSettingLabel = new DLabel(tr("User Setting："));
    userSettingLabel->setFixedWidth(120);
    d->userSettingEdit = new DLineEdit();
    DPushButton *userSettingBtn = new DPushButton(tr("Browse"));
    QHBoxLayout *userSettingLayout = new QHBoxLayout();
    userSettingLayout->addWidget(userSettingLabel);
    userSettingLayout->addWidget(d->userSettingEdit);
    userSettingLayout->addWidget(userSettingBtn);

    DLabel *localSettingLabel = new DLabel(tr("Local Setting："));
    localSettingLabel->setFixedWidth(120);
    d->localSettingEdit = new DLineEdit();
    DPushButton *localSettingBtn = new DPushButton(tr("Browse"));
    QHBoxLayout *localSettingLayout = new QHBoxLayout();
    localSettingLayout->addWidget(localSettingLabel);
    localSettingLayout->addWidget(d->localSettingEdit);
    localSettingLayout->addWidget(localSettingBtn);

    vLayout->addLayout(homePathLayout);
    vLayout->addLayout(userSettingLayout);
    vLayout->addLayout(localSettingLayout);
    vLayout->addStretch();


    QObject::connect(userSettingBtn, &DPushButton::clicked, [this](){
        DFileDialog fileDialog;
        QString filePath = fileDialog.getOpenFileName(nullptr, "Open Maven User Setting File", QString(), "*.xml");
        if(!filePath.isEmpty()) {
            d->userSettingEdit->setText(filePath);
        }
    });


    QObject::connect(localSettingBtn, &DPushButton::clicked, [this](){
        DFileDialog fileDialog;
        QString filePath = fileDialog.getExistingDirectory(nullptr, "Open Maven Local Setting Folder", QString(), DFileDialog::DontResolveSymlinks);
        if(!filePath.isEmpty()) {
            d->localSettingEdit->setText(filePath);
        }
    });
}

void MavenWidget::updateUi()
{
    const ToolChainData::ToolChains &data = d->toolChainData->getToolChanins();
    ToolChainData::Params cParams = data.value(kMaven);
    int i = 0;
    for (auto param : cParams) {
        QString text = param.name + "(" + param.path + ")";
        d->homePathComboBox->insertItem(i, text);
        d->homePathComboBox->setItemData(i, QVariant::fromValue(param), Qt::UserRole + 1);
        i++;
    }
}

bool MavenWidget::getControlValue(QMap<QString, QVariant> &map)
{
    MavenConfig config;
    int index = d->homePathComboBox->currentIndex();
    if (index < 0) {
        config.version = ToolChainData::ToolChainParam();
    } else {
        config.version = qvariant_cast<ToolChainData::ToolChainParam>(d->homePathComboBox->itemData(index, Qt::UserRole + 1));
    }

    config.userSetting = d->userSettingEdit->text();
    config.localSetting = d->localSettingEdit->text();

    dataToMap(config, map);

    return true;
}

void MavenWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    MavenConfig config;
    mapToData(map, config);

    int count = d->homePathComboBox->count();
    for (int i = 0; i < count; i++) {
        ToolChainData::ToolChainParam toolChainParam = qvariant_cast<ToolChainData::ToolChainParam>(d->homePathComboBox->itemData(i, Qt::UserRole + 1));
        if (config.version.name == toolChainParam.name
                && config.version.path == toolChainParam.path) {
            d->homePathComboBox->setCurrentIndex(i);
            break;
        }
    }

    d->userSettingEdit->setText(config.userSetting);
    d->localSettingEdit->setText(config.localSetting);
}

bool MavenWidget::dataToMap(const MavenConfig &config, QMap<QString, QVariant> &map)
{
    QMap<QString, QVariant> version;
    version.insert("name", config.version.name);
    version.insert("path", config.version.path);

    map.insert("version", version);
    map.insert("userSetting", config.userSetting);
    map.insert("localSetting", config.localSetting);

    return true;
}

bool MavenWidget::mapToData(const QMap<QString, QVariant> &map, MavenConfig &config)
{
    QMap<QString, QVariant> version = map.value("version").toMap();
    config.version.name = version.value("name").toString();
    config.version.path = version.value("path").toString();

    config.userSetting = map.value("userSetting").toString();
    config.localSetting = map.value("localSetting").toString();

    return true;
}

void MavenWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    setControlValue(map);
}

void MavenWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    getControlValue(map);
}
