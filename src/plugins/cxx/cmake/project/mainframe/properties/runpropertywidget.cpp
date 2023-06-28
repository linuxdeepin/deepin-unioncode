// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "runpropertywidget.h"
#include "environmentwidget.h"
#include "common/common.h"
#include "runconfigpane.h"
#include "configutil.h"
#include "targetsmanager.h"

#include <QListWidget>
#include <QSplitter>
#include <QComboBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QStyleFactory>
#include <QStackedWidget>

using namespace config;

class RunPropertyWidgetPrivate
{
    friend class RunPropertyWidget;

    QComboBox *exeComboBox{nullptr};
    RunConfigPane *runConfigPane{nullptr};

    QVector<RunParam> paramsShadow;
    QStandardItem *item{nullptr};
    dpfservice::ProjectInfo projectInfo;
};

RunPropertyWidget::RunPropertyWidget(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, QWidget *parent)
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

    QWidget *titleWidget = new QWidget(runCfgWidget);
    QHBoxLayout *runCfgLayout = new QHBoxLayout(titleWidget);
    QLabel *runCfgLabel = new QLabel(tr("Run configuration:"));
    d->exeComboBox = new QComboBox();
    d->exeComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QObject::connect(d->exeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        if (index >= 0 && index < d->paramsShadow.count()) {
            d->runConfigPane->bindValues(&d->paramsShadow[index]);
        }
    });
    runCfgLayout->addWidget(runCfgLabel, 0, Qt::AlignLeft);
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
