/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhouyi<zhouyi1@uniontech.com>
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
};

RunPropertyWidget::RunPropertyWidget(QWidget *parent)
    : PageWidget(parent)
    , d(new RunPropertyWidgetPrivate())
{
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

    QString filePath = ConfigUtil::instance()->getConfigPath(QFileInfo(param->projectPath).path());
    ConfigUtil::instance()->saveConfig(filePath, *param);
}
