// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "runpropertypage.h"
#include "environmentwidget.h"
#include "common/common.h"
#include "runconfigpane.h"
#include "configutil.h"
#include "targetsmanager.h"
#include "services/project/projectservice.h"

#include <DListWidget>
#include <DSplitter>
#include <DComboBox>
#include <DGroupBox>
#include <DPushButton>
#include <DStackedWidget>

#include <QVBoxLayout>
#include <QStyleFactory>

using namespace config;
using namespace dpfservice;

class RunPropertyWidgetPrivate
{
    friend class RunPropertyPage;

    DComboBox *exeComboBox{nullptr};
    RunConfigPane *runConfigPane{nullptr};

    QVector<TargetRunConfigure> targetsRunConfigure;
    QStandardItem *item{nullptr};
    dpfservice::ProjectInfo projectInfo;
};

RunPropertyPage::RunPropertyPage(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, DWidget *parent)
    : PageWidget(parent)
    , d(new RunPropertyWidgetPrivate())
{
    d->projectInfo = projectInfo;
    d->item = item;
    setupUi();

    connect(ConfigUtil::instance(), &ConfigUtil::configTypeSwitched, this, [=]() {
        updateData();
    });
}

RunPropertyPage::~RunPropertyPage()
{
    if (d)
        delete d;
}

void RunPropertyPage::setupUi()
{
    ConfigureWidget *runCfgWidget = new ConfigureWidget(this);
    runCfgWidget->setFrameShape(QFrame::Shape::NoFrame);

    DLabel *runCfgLabel = new DLabel(tr("Run configuration:"), this);
    d->exeComboBox = new DComboBox();
    d->exeComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->exeComboBox->setFixedWidth(220);
    QObject::connect(d->exeComboBox, QOverload<int>::of(&DComboBox::currentIndexChanged), [=](int index){
        if (index >= 0 && index < d->targetsRunConfigure.count()) {
            d->runConfigPane->setTargetRunParam(&d->targetsRunConfigure[index]);
        }
    });

    d->runConfigPane = new RunConfigPane(this);
    d->runConfigPane->insertTitle(runCfgLabel, d->exeComboBox);
    runCfgWidget->addWidget(d->runConfigPane);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(runCfgWidget);
}

void RunPropertyPage::updateData()
{
    d->exeComboBox->clear();

    ProjectConfigure *param = ConfigUtil::instance()->getConfigureParamPointer();
    for (auto iter = param->buildTypeConfigures.begin(); iter != param->buildTypeConfigures.end(); ++iter) {
        if (param->tempSelType == iter->type) {
            d->targetsRunConfigure = iter->runConfigure.targetsRunConfigure;
            auto iterExe = d->targetsRunConfigure.begin();
            int index = 0;
            for (; iterExe != d->targetsRunConfigure.end(); ++iterExe, index++) {
                d->exeComboBox->insertItem(index, iterExe->targetName);
                if (iter->runConfigure.defaultTargetName == iterExe->targetName) {
                    d->exeComboBox->setCurrentIndex(index);
                    d->runConfigPane->setTargetRunParam(&(*iterExe));
                }
            }
            break;
        }
    }
}

void RunPropertyPage::readConfig()
{
    updateData();
}

void RunPropertyPage::saveConfig()
{
    ProjectConfigure *param = ConfigUtil::instance()->getConfigureParamPointer();

    auto iter = param->buildTypeConfigures.begin();
    for (; iter != param->buildTypeConfigures.end(); ++iter) {
        if (param->defaultType == iter->type) {
            iter->runConfigure.targetsRunConfigure = d->targetsRunConfigure;
            iter->runConfigure.defaultTargetName = d->projectInfo.currentProgram();
        }
        if (param->tempSelType == iter->type) {
            auto activeTarget = TargetsManager::instance()->getActivedTargetByTargetType(kActiveExecTarget);
            for (auto it : iter->runConfigure.targetsRunConfigure) {
                if (it.targetName == activeTarget.name) {
                    d->projectInfo.setRunEnvironment(it.env.toList());
                    break;
                }
            }
        }
    }

    QString filePath = ConfigUtil::instance()->getConfigPath(param->workspace);
    ConfigUtil::instance()->saveConfig(filePath, *param);

    ConfigUtil::instance()->updateProjectInfo(d->projectInfo, param);
    dpfservice::ProjectInfo::set(d->item, d->projectInfo);
}
