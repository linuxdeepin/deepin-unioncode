/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
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
#include "mavenwidget.h"
#include "services/option/toolchaindata.h"
#include "common/util/custompaths.h"

#include <QRadioButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QDebug>
#include <QFileDialog>

class MavenWidgetPrivate
{
    friend class MavenWidget;

    QComboBox *homePathComboBox = nullptr;
    QLineEdit *userSettingEdit = nullptr;
    QLineEdit *localSettingEdit = nullptr;
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

    QLabel *homePathLabel = new QLabel(QLabel::tr("Maven path："));
    homePathLabel->setFixedWidth(120);
    d->homePathComboBox = new QComboBox();

    QHBoxLayout *homePathLayout = new QHBoxLayout();
    homePathLayout->addWidget(homePathLabel);
    homePathLayout->addWidget(d->homePathComboBox);

    QLabel *userSettingLabel = new QLabel(QLabel::tr("User Setting："));
    userSettingLabel->setFixedWidth(120);
    d->userSettingEdit = new QLineEdit();
    QPushButton *userSettingBtn = new QPushButton(QPushButton::tr("Browse"));
    QHBoxLayout *userSettingLayout = new QHBoxLayout();
    userSettingLayout->addWidget(userSettingLabel);
    userSettingLayout->addWidget(d->userSettingEdit);
    userSettingLayout->addWidget(userSettingBtn);

    QLabel *localSettingLabel = new QLabel(QLabel::tr("Local Setting："));
    localSettingLabel->setFixedWidth(120);
    d->localSettingEdit = new QLineEdit();
    QPushButton *localSettingBtn = new QPushButton(QPushButton::tr("Browse"));
    QHBoxLayout *localSettingLayout = new QHBoxLayout();
    localSettingLayout->addWidget(localSettingLabel);
    localSettingLayout->addWidget(d->localSettingEdit);
    localSettingLayout->addWidget(localSettingBtn);

    vLayout->addLayout(homePathLayout);
    vLayout->addLayout(userSettingLayout);
    vLayout->addLayout(localSettingLayout);
    vLayout->addStretch();


    QObject::connect(userSettingBtn, &QPushButton::clicked, [this](){
        QFileDialog fileDialog;
        QString filePath = fileDialog.getOpenFileName(nullptr, "Open Maven User Setting File", QString(), "*.xml");
        if(!filePath.isEmpty()) {
            d->userSettingEdit->setText(filePath);
        }
    });


    QObject::connect(localSettingBtn, &QPushButton::clicked, [this](){
        QFileDialog fileDialog;
        QString filePath = fileDialog.getExistingDirectory(nullptr, "Open Maven Local Setting Folder", QString(), QFileDialog::DontResolveSymlinks);
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
        return false;
    }

    config.version = qvariant_cast<ToolChainData::ToolChainParam>(d->homePathComboBox->itemData(index, Qt::UserRole + 1));
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
