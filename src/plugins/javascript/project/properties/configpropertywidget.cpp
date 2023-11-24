// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configpropertywidget.h"

#include "services/option/toolchaindata.h"
#include "common/toolchain/toolchain.h"
#include "common/widget/pagewidget.h"

#include <DComboBox>
#include <DLabel>
#include <DLineEdit>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace config;

class DetailPropertyWidgetPrivate
{
    friend class DetailPropertyWidget;
    DComboBox *pyVersionComboBox{nullptr};
    QSharedPointer<ToolChainData> toolChainData;
};

DetailPropertyWidget::DetailPropertyWidget(QWidget *parent)
    : DWidget(parent)
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
    DLabel *label = new DLabel(DLabel::tr("JS interpreter: "));
    label->setFixedWidth(120);
    d->pyVersionComboBox = new DComboBox();
    hLayout->addWidget(label);
    hLayout->addWidget(d->pyVersionComboBox);
    vLayout->addLayout(hLayout);

    vLayout->addStretch(10);
}

void DetailPropertyWidget::initData()
{
    d->toolChainData.reset(new ToolChainData());
    QString retMsg;
    bool ret = d->toolChainData->readToolChainData(retMsg);
    if (ret) {
        const ToolChainData::ToolChains &data = d->toolChainData->getToolChanins();
        auto initComboBox = [](DComboBox *comboBox, const ToolChainData::ToolChains &data, const QString &type) {
            int index = 0;
            ToolChainData::Params params = data.value(type);
            for (auto param : params) {
                QString text = param.name + "(" + param.path + ")";
                comboBox->insertItem(index, text);
                comboBox->setItemData(index, QVariant::fromValue(param), Qt::UserRole + 1);
                index++;
            }
        };

        initComboBox(d->pyVersionComboBox, data, kJS);
    }
}

void DetailPropertyWidget::setValues(const config::ConfigureParam *param)
{
    if (!param)
        return;

    auto initComboBox = [](DComboBox *comboBox, const config::ItemInfo &itemInfo) {
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

    initComboBox(d->pyVersionComboBox, param->jsVersion);
}

void DetailPropertyWidget::getValues(config::ConfigureParam *param)
{
    if (!param)
        return;

    auto getValue = [](DComboBox *comboBox, ItemInfo &itemInfo){
        itemInfo.clear();
        int index = comboBox->currentIndex();
        if (index > -1) {
            ToolChainData::ToolChainParam value = qvariant_cast<ToolChainData::ToolChainParam>(comboBox->itemData(index, Qt::UserRole + 1));
            itemInfo.name = value.name;
            itemInfo.path = value.path;
        }
    };

    getValue(d->pyVersionComboBox, param->jsVersion);
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
