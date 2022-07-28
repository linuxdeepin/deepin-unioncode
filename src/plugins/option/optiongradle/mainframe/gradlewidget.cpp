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
#include "gradlewidget.h"
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

class GradleWidgetPrivate
{
    friend class GradleWidget;

    QRadioButton *useWrapper = nullptr;
    QRadioButton *useLocal = nullptr;
    QComboBox *localDetail = nullptr;
    QSharedPointer<ToolChainData> toolChainData;
};

GradleWidget::GradleWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new GradleWidgetPrivate())
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

GradleWidget::~GradleWidget()
{
    if (d) {
        delete d;
    }
}

void GradleWidget::setupUi()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    QLabel *label = new QLabel(QLabel::tr("Gradle distribution："));
    d->useWrapper = new QRadioButton("use Gradle wrapper");
    d->useWrapper->setChecked(true);

    QHBoxLayout *localLayout = new QHBoxLayout();
    d->useLocal = new QRadioButton("use Local installation, directory：");
    d->useLocal->setFixedWidth(300);
    d->localDetail = new QComboBox();
    localLayout->addWidget(d->useLocal);
    localLayout->addWidget(d->localDetail);

    QObject::connect(d->useLocal, &QRadioButton::setChecked, [this](){
        d->localDetail->setEnabled(d->useLocal->isChecked());
    });

    vLayout->addWidget(label);
    vLayout->addWidget(d->useWrapper);
    vLayout->addLayout(localLayout);
    vLayout->addStretch();
}

void GradleWidget::updateUi()
{
    const ToolChainData::ToolChains &data = d->toolChainData->getToolChanins();
    ToolChainData::Params cParams = data.value(kGradle);
    int i = 0;
    for (auto param : cParams) {
        QString text = param.name + "(" + param.path + ")";
        d->localDetail->insertItem(i, text);
        d->localDetail->setItemData(i, QVariant::fromValue(param), Qt::UserRole + 1);
        i++;
    }
}

bool GradleWidget::getControlValue(QMap<QString, QVariant> &map)
{
    GradleConfig config;
    config.useWrapper = d->useWrapper->isChecked() ? "1" : "0";
    config.useLocal = d->useLocal->isChecked() ? "1" : "0";

    int index = d->localDetail->currentIndex();
    if (index < 0) {
        config.version = ToolChainData::ToolChainParam();
    } else {
        config.version = qvariant_cast<ToolChainData::ToolChainParam>(d->localDetail->itemData(index, Qt::UserRole + 1));
    }

    dataToMap(config, map);

    return true;
}

void GradleWidget::setControlValue(const QMap<QString, QVariant> &map)
{
    GradleConfig config;
    mapToData(map, config);

    int count = d->localDetail->count();
    for (int i = 0; i < count; i++) {
        ToolChainData::ToolChainParam toolChainParam = qvariant_cast<ToolChainData::ToolChainParam>(d->localDetail->itemData(i, Qt::UserRole + 1));
        if (config.version.name == toolChainParam.name
                && config.version.path == toolChainParam.path) {
            d->localDetail->setCurrentIndex(i);
            break;
        }
    }

    d->useWrapper->setChecked(config.useWrapper.toInt() == 1);
    d->useLocal->setChecked(config.useLocal.toInt() == 1);
}

bool GradleWidget::dataToMap(const GradleConfig &config, QMap<QString, QVariant> &map)
{
    QMap<QString, QVariant> version;
    version.insert("name", config.version.name);
    version.insert("path", config.version.path);

    map.insert("version", version);
    map.insert("useWrapper", config.useWrapper);
    map.insert("useLocal", config.useLocal);

    return true;
}

bool GradleWidget::mapToData(const QMap<QString, QVariant> &map, GradleConfig &config)
{
    QMap<QString, QVariant> version = map.value("version").toMap();
    config.version.name = version.value("name").toString();
    config.version.path = version.value("path").toString();

    config.useWrapper = map.value("useWrapper").toString();
    config.useLocal = map.value("useLocal").toString();

    return true;
}

void GradleWidget::setUserConfig(const QMap<QString, QVariant> &map)
{
    setControlValue(map);
}

void GradleWidget::getUserConfig(QMap<QString, QVariant> &map)
{
    getControlValue(map);
}
