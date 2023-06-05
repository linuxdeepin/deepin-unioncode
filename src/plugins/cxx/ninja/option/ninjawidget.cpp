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
#include "ninjawidget.h"
#include "services/option/toolchaindata.h"
#include "common/util/custompaths.h"
#include "common/toolchain/toolchain.h"

#include <QRadioButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDir>
#include <QDebug>
#include <QJsonObject>

class NinjaWidgetPrivate
{
    friend class NinjaWidget;

    QComboBox *homePathComboBox = nullptr;
    QSharedPointer<ToolChainData> toolChainData;
};

NinjaWidget::NinjaWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new NinjaWidgetPrivate())
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

NinjaWidget::~NinjaWidget()
{
    if (d) {
        delete d;
    }
}

void NinjaWidget::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    QLabel *homePathLabel = new QLabel(QLabel::tr("Ninja path："));
    homePathLabel->setFixedWidth(120);
    d->homePathComboBox = new QComboBox();

    QHBoxLayout *homePathLayout = new QHBoxLayout();
    homePathLayout->addWidget(homePathLabel);
    homePathLayout->addWidget(d->homePathComboBox);

    vLayout->addLayout(homePathLayout);
    vLayout->addStretch();
}

void NinjaWidget::updateUi()
{
    const ToolChainData::ToolChains &data = d->toolChainData->getToolChanins();
    ToolChainData::Params cParams = data.value(kNinja);
    int i = 0;
    for (auto param : cParams) {
        QString text = param.name + "(" + param.path + ")";
        d->homePathComboBox->insertItem(i, text);
        d->homePathComboBox->setItemData(i, QVariant::fromValue(param), Qt::UserRole + 1);
        i++;
    }
}

bool NinjaWidget::getControlValue(QMap<QString, QVariant> &map)
{
    NinjaConfig config;
    int index = d->homePathComboBox->currentIndex();
    if (index < 0) {
        config.version = ToolChainData::ToolChainParam();
    } else {
        config.version = qvariant_cast<ToolChainData::ToolChainParam>(d->homePathComboBox->itemData(index, Qt::UserRole + 1));
    }

    dataToMap(config, map);

    return true;
}

void NinjaWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    NinjaConfig config;
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
}

bool NinjaWidget::dataToMap(const NinjaConfig &config, QMap<QString, QVariant> &map)
{
    QMap<QString, QVariant> version;
    version.insert("name", config.version.name);
    version.insert("path", config.version.path);

    map.insert("version", version);

    return true;
}

bool NinjaWidget::mapToData(const QMap<QString, QVariant> &map, NinjaConfig &config)
{
    QMap<QString, QVariant> version = map.value("version").toMap();
    config.version.name = version.value("name").toString();
    config.version.path = version.value("path").toString();

    return true;
}

void NinjaWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    setControlValue(map);
}

void NinjaWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    getControlValue(map);
}
