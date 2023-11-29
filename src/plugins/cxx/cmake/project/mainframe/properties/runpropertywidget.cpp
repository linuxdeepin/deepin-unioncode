// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "runpropertywidget.h"
#include "environmentwidget.h"
#include "common/common.h"
#include "runconfigpane.h"
#include "configutil.h"
#include "targetsmanager.h"

#include <DListWidget>
#include <DSplitter>
#include <DComboBox>
#include <DGroupBox>
#include <DPushButton>
#include <DStackedWidget>

#include <QVBoxLayout>
#include <QStyleFactory>

using namespace config;

class RunPropertyWidgetPrivate
{
    friend class RunPropertyWidget;

    DComboBox *exeComboBox{nullptr};
    RunConfigPane *runConfigPane{nullptr};

    QVector<RunParam> paramsShadow;
    QStandardItem *item{nullptr};
    dpfservice::ProjectInfo projectInfo;
};

RunPropertyWidget::RunPropertyWidget(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, DWidget *parent)
    : PageWidget(parent)
    , d(new RunPropertyWidgetPrivate())
{
    d->projectInfo = projectInfo;
    d->item = item;
    setupUi();
}

RunPropertyWidget::~RunPropertyWidget()
{
    if (d)
        delete d;
}

void RunPropertyWidget::setupUi()
{
    ConfigureWidget *runCfgWidget = new ConfigureWidget(this);
    runCfgWidget->setFrameShape(QFrame::Shape::NoFrame);

    DWidget *titleWidget = new DWidget(runCfgWidget);
    QHBoxLayout *runCfgLayout = new QHBoxLayout(titleWidget);
    DLabel *runCfgLabel = new DLabel(tr("Run configuration:"));
    runCfgLabel->setContentsMargins(10, 0, 0, 0);

    d->exeComboBox = new DComboBox();
    d->exeComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QObject::connect(d->exeComboBox, QOverload<int>::of(&DComboBox::currentIndexChanged), [=](int index){
        if (index >= 0 && index < d->paramsShadow.count()) {
            d->runConfigPane->bindValues(&d->paramsShadow[index]);
        }
    });
    runCfgLayout->addWidget(runCfgLabel, 0, Qt::AlignLeft);
    runCfgLayout->addStretch(2);
    runCfgLayout->addWidget(d->exeComboBox, 0, Qt::AlignLeft);
    runCfgLayout->addStretch(10);
    runCfgLayout->setMargin(0);

    d->runConfigPane = new RunConfigPane(this);
    runCfgWidget->addWidget(titleWidget);
    runCfgWidget->addWidget(d->runConfigPane);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(runCfgWidget);
}

void RunPropertyWidget::updateData()
{
    d->exeComboBox->clear();

    ConfigureParam *param = ConfigUtil::instance()->getConfigureParamPointer();
    for (auto iter = param->buildConfigures.begin(); iter != param->buildConfigures.end(); ++iter) {
        if (param->tempSelType == iter->type) {
            d->paramsShadow = iter->runConfigure.params;
            auto iterExe = d->paramsShadow.begin();
            int index = 0;
            for (; iterExe != d->paramsShadow.end(); ++iterExe, index++) {
                d->exeComboBox->insertItem(index, iterExe->targetName);
                if (iter->runConfigure.defaultTargetName == iterExe->targetName) {
                    d->exeComboBox->setCurrentIndex(index);
                    d->runConfigPane->bindValues(iterExe);
                }
            }
            break;
        }
    }
}

void RunPropertyWidget::readConfig()
{
    updateData();
}

void RunPropertyWidget::saveConfig()
{
    ConfigureParam *param = ConfigUtil::instance()->getConfigureParamPointer();
    auto iter = param->buildConfigures.begin();
    for (; iter != param->buildConfigures.end(); ++iter) {
        if (param->defaultType == iter->type) {
            iter->runConfigure.params = d->paramsShadow;
            iter->runConfigure.defaultTargetName = d->exeComboBox->currentText();
        }
    }

    QString filePath = ConfigUtil::instance()->getConfigPath(param->workspace);
    ConfigUtil::instance()->saveConfig(filePath, *param);

    ConfigUtil::instance()->updateProjectInfo(d->projectInfo, param);
    dpfservice::ProjectInfo::set(d->item, d->projectInfo);
}
