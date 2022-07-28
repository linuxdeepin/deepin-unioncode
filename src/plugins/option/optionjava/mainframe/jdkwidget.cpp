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
#include "jdkwidget.h"
#include "services/option/toolchaindata.h"
#include "common/util/custompaths.h"
#include "common/toolchain/toolchain.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QFile>
#include <QJsonArray>
#include <QDebug>
#include <QDir>

#include <QRadioButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDir>


class JDKWidgetPrivate
{
    friend class JDKWidget;

    QComboBox *jdkDetail = nullptr;
    QSharedPointer<ToolChainData> toolChainData;
};

JDKWidget::JDKWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new JDKWidgetPrivate())
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

JDKWidget::~JDKWidget()
{
    if (d) {
        delete d;
    }
}

void JDKWidget::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *label = new QLabel(QLabel::tr("JDK version："));
    label->setFixedWidth(120);
    d->jdkDetail = new QComboBox();
    hLayout->addWidget(label);
    hLayout->addWidget(d->jdkDetail);

    vLayout->addLayout(hLayout);
    vLayout->addStretch();
}

void JDKWidget::updateUi()
{
    const ToolChainData::ToolChains &data = d->toolChainData->getToolChanins();
    ToolChainData::Params cParams = data.value(kJDK);
    int i = 0;
    for (auto param : cParams) {
        QString text = param.name + "(" + param.path + ")";
        d->jdkDetail->insertItem(i, text);
        d->jdkDetail->setItemData(i, QVariant::fromValue(param), Qt::UserRole + 1);
        i++;
    }
}

bool JDKWidget::getControlValue(QMap<QString, QVariant> &map)
{
    JDKConfig config;
    int index = d->jdkDetail->currentIndex();
    if (index < 0) {
        config.version = ToolChainData::ToolChainParam();
    } else {
        config.version = qvariant_cast<ToolChainData::ToolChainParam>(d->jdkDetail->itemData(index, Qt::UserRole + 1));
    }

    dataToMap(config, map);

    return true;
}

void JDKWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    JDKConfig config;
    mapToData(map, config);

    int count = d->jdkDetail->count();
    for (int i = 0; i < count; i++) {
        ToolChainData::ToolChainParam toolChainParam = qvariant_cast<ToolChainData::ToolChainParam>(d->jdkDetail->itemData(i, Qt::UserRole + 1));
        if (config.version.name == toolChainParam.name
                && config.version.path == toolChainParam.path) {
            d->jdkDetail->setCurrentIndex(i);
            break;
        }
    }
}

bool JDKWidget::dataToMap(const JDKConfig &config, QMap<QString, QVariant> &map)
{
    QMap<QString, QVariant> version;
    version.insert("name", config.version.name);
    version.insert("path", config.version.path);

    map.insert("version", version);

    return true;
}

bool JDKWidget::mapToData(const QMap<QString, QVariant> &map, JDKConfig &config)
{
    QMap<QString, QVariant> version = map.value("version").toMap();
    config.version.name = version.value("name").toString();
    config.version.path = version.value("path").toString();

    return true;
}

void JDKWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    setControlValue(map);
}

void JDKWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    getControlValue(map);
}
