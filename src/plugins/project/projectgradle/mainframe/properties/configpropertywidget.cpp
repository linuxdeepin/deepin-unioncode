/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhouyi<zhouyi1@uniontech.com>
 *
 * Maintainer: zhouyi<zhouyi1@uniontech.com>
 *
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
#include "configpropertywidget.h"

#include "services/option/toolchaindata.h"
#include "common/toolchain/toolchain.h"
#include "common/widget/pagewidget.h"

#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

using namespace config;

class DetailPropertyWidgetPrivate
{
    friend class DetailPropertyWidget;
    QComboBox *jdkVersionComboBox{nullptr};
    QComboBox *gradleVersionComboBox{nullptr};
    QLineEdit *mainClass{nullptr};
    QSharedPointer<ToolChainData> toolChainData;
    QCheckBox *detailBox{nullptr};
};

DetailPropertyWidget::DetailPropertyWidget(QWidget *parent)
    : QWidget(parent)
    , d(new DetailPropertyWidgetPrivate())
{
    setupUI();
    initData();
}

DetailPropertyWidget::~DetailPropertyWidget()
{
    if (d)
        delete d;
}

void DetailPropertyWidget::setupUI()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    QHBoxLayout *hLayout = new QHBoxLayout();
    QLabel *label = new QLabel(QLabel::tr("JDK version："));
    label->setFixedWidth(120);
    d->jdkVersionComboBox = new QComboBox();
    hLayout->addWidget(label);
    hLayout->addWidget(d->jdkVersionComboBox);
    vLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout();
    label = new QLabel(QLabel::tr("Gradle Version: "));
    label->setFixedWidth(120);
    d->gradleVersionComboBox = new QComboBox();
    hLayout->addWidget(label);
    hLayout->addWidget(d->gradleVersionComboBox);
    vLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout();
    label = new QLabel(QLabel::tr("Main Class："));
    label->setFixedWidth(120);
    d->mainClass = new QLineEdit();
    d->mainClass->setPlaceholderText(tr("Input main class"));
    hLayout->addWidget(label);
    hLayout->addWidget(d->mainClass);
    vLayout->addLayout(hLayout);
    vLayout->addStretch(10);

    hLayout = new QHBoxLayout();
    label = new QLabel(QLabel::tr("Detail output:"));
    label->setFixedWidth(120);
    d->detailBox = new QCheckBox();
    hLayout->addWidget(label);
    hLayout->addWidget(d->detailBox);
    hLayout->setAlignment(Qt::AlignLeft);
    vLayout->addLayout(hLayout);
}

void DetailPropertyWidget::initData()
{
    d->toolChainData.reset(new ToolChainData());
    QString retMsg;
    bool ret = d->toolChainData->readToolChainData(retMsg);
    if (ret) {
        const ToolChainData::ToolChains &data = d->toolChainData->getToolChanins();
        auto initComboBox = [](QComboBox *comboBox, const ToolChainData::ToolChains &data, const QString &type) {
            int index = 0;
            ToolChainData::Params params = data.value(type);
            for (auto param : params) {
                QString text = param.name + "(" + param.path + ")";
                comboBox->insertItem(index, text);
                comboBox->setItemData(index, QVariant::fromValue(param), Qt::UserRole + 1);
                index++;
            }
        };

        initComboBox(d->jdkVersionComboBox, data, kJDK);
        initComboBox(d->gradleVersionComboBox, data, kGradle);
    }
}

void DetailPropertyWidget::setValues(const config::ConfigureParam *param)
{
    if (!param)
        return;

    auto initComboBox = [](QComboBox *comboBox, const config::ItemInfo &itemInfo) {
        int count = comboBox->count();
        for (int i = 0; i < count; i++) {
            ToolChainData::ToolChainParam toolChainParam = qvariant_cast<ToolChainData::ToolChainParam>(comboBox->itemData(i, Qt::UserRole + 1));
            if (itemInfo.name == toolChainParam.name
                    && itemInfo.path == toolChainParam.path) {
                comboBox->setCurrentIndex(i);
                break;
            }
        }
    };

    initComboBox(d->jdkVersionComboBox, param->jdkVersion);
    initComboBox(d->gradleVersionComboBox, param->gradleVersion);
    d->mainClass->setText(param->mainClass);
    d->detailBox->setChecked(param->detailInfo);
}

void DetailPropertyWidget::getValues(config::ConfigureParam *param)
{
    if (!param)
        return;

    auto getValue = [](QComboBox *comboBox, ItemInfo &itemInfo){
        itemInfo.clear();
        int index = comboBox->currentIndex();
        if (index > -1) {
            ToolChainData::ToolChainParam value = qvariant_cast<ToolChainData::ToolChainParam>(comboBox->itemData(index, Qt::UserRole + 1));
            itemInfo.name = value.name;
            itemInfo.path = value.path;
        }
    };

    getValue(d->jdkVersionComboBox, param->jdkVersion);
    getValue(d->gradleVersionComboBox, param->gradleVersion);
    param->mainClass = d->mainClass->text();
    param->detailInfo = d->detailBox->isChecked();
}

class ConfigPropertyWidgetPrivate
{
    friend class ConfigPropertyWidget;

    DetailPropertyWidget *detail{nullptr};
    QStandardItem *item{nullptr};
    dpfservice::ProjectInfo projectInfo;
};

ConfigPropertyWidget::ConfigPropertyWidget(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, QWidget *parent)
    : PageWidget(parent)
    , d(new ConfigPropertyWidgetPrivate())
{
    d->item = item;
    d->projectInfo = projectInfo;
    setupUI();
    initData(projectInfo);
}

ConfigPropertyWidget::~ConfigPropertyWidget()
{
    if (d)
        delete d;
}

void ConfigPropertyWidget::setupUI()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    d->detail = new DetailPropertyWidget();
    vLayout->addWidget(d->detail);
    vLayout->addStretch(10);
}

void ConfigPropertyWidget::initData(const dpfservice::ProjectInfo &projectInfo)
{
    ConfigureParam *param = ConfigUtil::instance()->getConfigureParamPointer();
    ConfigUtil::instance()->readConfig(ConfigUtil::instance()->getConfigPath(projectInfo.workspaceFolder()), *param);
    d->detail->setValues(param);
    param->kit = projectInfo.kitName();
    param->language = projectInfo.language();
    param->projectPath = projectInfo.workspaceFolder();
}

void ConfigPropertyWidget::saveConfig()
{
    ConfigureParam *param = ConfigUtil::instance()->getConfigureParamPointer();
    d->detail->getValues(param);

    QString filePath = ConfigUtil::instance()->getConfigPath(param->projectPath);
    ConfigUtil::instance()->saveConfig(filePath, *param);

    ConfigUtil::instance()->updateProjectInfo(d->projectInfo, param);
    dpfservice::ProjectInfo::set(d->item, d->projectInfo);
}
