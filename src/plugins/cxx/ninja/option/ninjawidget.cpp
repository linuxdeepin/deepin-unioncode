// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ninjawidget.h"
#include "services/option/toolchaindata.h"
#include "common/util/custompaths.h"
#include "common/toolchain/toolchain.h"

#include <DRadioButton>
#include <DComboBox>
#include <DLabel>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDir>
#include <QDebug>
#include <QJsonObject>

DWIDGET_USE_NAMESPACE

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

    DLabel *homePathLabel = new DLabel(DLabel::tr("Ninja path："));
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
