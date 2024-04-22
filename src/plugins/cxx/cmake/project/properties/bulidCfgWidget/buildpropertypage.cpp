// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buildpropertypage.h"

#include "common/common.h"
#include "environmentwidget.h"
#include "stepspane.h"
#include "targetsmanager.h"
#include "services/project/projectservice.h"
#include "cmakeCfgWidget/cmakepropertypage.h"

#include <DTabWidget>
#include <DPushButton>
#include <DFileDialog>
#include <DStackedWidget>
#include <DComboBox>
#include <DButtonBox>
#include <DFrame>
#include <DLineEdit>

#include <QVBoxLayout>
#include <QFormLayout>

DWIDGET_USE_NAMESPACE
using namespace config;
using namespace dpfservice;

class DetailPropertyWidgetPrivate
{
    friend class DetailPropertyWidget;

    StepsPane *buildStepsPane{nullptr};
    StepsPane *cleanStepsPane{nullptr};
    EnvironmentWidget *envWidget{nullptr};
    CMakePropertyPage *cmakeWidget{nullptr};
};

DetailPropertyWidget::DetailPropertyWidget(QWidget *parent)
    : ConfigureWidget(parent)
    , d(new DetailPropertyWidgetPrivate())
{
    setBackgroundRole(QPalette::Window);
    setFrameShape(QFrame::Shape::NoFrame);

    d->buildStepsPane = new StepsPane(this);
    d->cleanStepsPane = new StepsPane(this);
    d->envWidget = new EnvironmentWidget(this);
    d->cmakeWidget = new CMakePropertyPage(this);

    DStackedWidget *stackWidget = new DStackedWidget(this);
    stackWidget->insertWidget(0, d->buildStepsPane);
    stackWidget->insertWidget(1, d->cleanStepsPane);
    stackWidget->insertWidget(2, d->envWidget);
    stackWidget->insertWidget(3, d->cmakeWidget);

    auto buildPage = static_cast<BuildPropertyPage *>(parentWidget());
    if (buildPage)
        connect(d->cmakeWidget, &CMakePropertyPage::cacheFileUpdated, buildPage, &BuildPropertyPage::cacheFileUpdated);

    DButtonBoxButton *btnBuild = new DButtonBoxButton(QObject::tr("Build Steps"), this);
    btnBuild->setCheckable(true);
    btnBuild->setChecked(true);
    DButtonBoxButton *btnClean = new DButtonBoxButton(QObject::tr("Clean Steps"), this);
    DButtonBoxButton *btnEnv = new DButtonBoxButton(QObject::tr("Runtime Env"), this);
    DButtonBoxButton *btnCMake = new DButtonBoxButton(QObject::tr("CMake config"), this);

    DButtonBox *btnbox = new DButtonBox(this);
    QList<DButtonBoxButton *> list { btnBuild, btnClean, btnEnv, btnCMake};
    btnbox->setButtonList(list, true);

    auto frame = new DWidget(this);
    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignHCenter);
    layout->addWidget(btnbox);
    frame->setLayout(layout);

    connect(btnbox, &DButtonBox::buttonClicked, this, [=](QAbstractButton *button) {
        if (button == btnBuild)
            stackWidget->setCurrentIndex(0);
        else if (button == btnClean)
            stackWidget->setCurrentIndex(1);
        else if (button == btnEnv)
            stackWidget->setCurrentIndex(2);
        else if (button == btnCMake)
            stackWidget->setCurrentIndex(3);
    });

    addWidget(frame);
    addWidget(stackWidget);
}

DetailPropertyWidget::~DetailPropertyWidget()
{
    if (d)
        delete d;
}

void DetailPropertyWidget::setValues(const BuildTypeConfigure &configure)
{
    auto &steps = configure.buildConfigure.steps;
    for(auto iter = steps.begin(); iter != steps.end(); ++iter) {
        if (iter->type == Build)
            d->buildStepsPane->setValues(*iter);
        else if (iter->type == Clean)
            d->cleanStepsPane->setValues(*iter);
    }

    auto cacheFile = configure.directory + QDir::separator() + "CMakeCache.txt";
    if (QFileInfo(cacheFile).exists())
        d->cmakeWidget->getItemsFromCacheFile(cacheFile);

    d->envWidget->setValues(configure.buildConfigure.env);
}

void DetailPropertyWidget::getValues(BuildTypeConfigure &configure)
{
    auto &steps = configure.buildConfigure.steps;
    for(auto iter = steps.begin(); iter != steps.end(); ++iter) {
        if (iter->type == Build)
            d->buildStepsPane->getValues(*iter);
        else if (iter->type == Clean)
            d->cleanStepsPane->getValues(*iter);
    }

    d->envWidget->getValues(configure.buildConfigure.env);

    auto cacheFile = configure.directory + QDir::separator() + "CMakeCache.txt";
    if (QFileInfo(cacheFile).exists()) {
        d->cmakeWidget->saveConfigToCacheFile(cacheFile);
    }
}

class BuildPropertyWidgetPrivate
{
    friend class BuildPropertyPage;

    DComboBox *configureComboBox{nullptr};
    DLineEdit *outputDirEdit{nullptr};

    DStackedWidget* stackWidget{nullptr};
    dpfservice::ProjectInfo projectInfo;

    QMap<StepType, dpfservice::TargetType> typeMap = {{StepType::Build, dpfservice::TargetType::kBuildTarget},
                                                      {StepType::Build, dpfservice::TargetType::kBuildTarget}};
};

BuildPropertyPage::BuildPropertyPage(const dpfservice::ProjectInfo &projectInfo, QWidget *parent)
    : PageWidget(parent)
    , d(new BuildPropertyWidgetPrivate())
{
    d->projectInfo = projectInfo;
    setupOverviewUI();
    initData(projectInfo);

    QObject::connect(TargetsManager::instance(), &TargetsManager::initialized,
                     this, &BuildPropertyPage::updateDetail);
}

BuildPropertyPage::~BuildPropertyPage()
{
    if (d)
        delete d;
}

void BuildPropertyPage::setupOverviewUI()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    ConfigureWidget *buildCfgWidget = new ConfigureWidget(this);
    buildCfgWidget->setFrameShape(QFrame::Shape::NoFrame);
    vLayout->addWidget(buildCfgWidget);
    setLayout(vLayout);

    QVBoxLayout *overviewLayout = new QVBoxLayout();
    DWidget *overviewWidget = new DWidget(this);
    overviewWidget->setLayout(overviewLayout);

    QHBoxLayout *configureLayout = new QHBoxLayout();
    d->configureComboBox = new DComboBox(this);
    configureLayout->addWidget(d->configureComboBox);
    configureLayout->setSpacing(10);
    configureLayout->addStretch();
    QObject::connect(d->configureComboBox, QOverload<int>::of(&DComboBox::currentIndexChanged), [=](int index){
        QVariant var = d->configureComboBox->itemData(index, Qt::UserRole + 1);
        if (var.isValid()) {
            QString directory = var.value<QString>();
            if (d->outputDirEdit) {
                d->outputDirEdit->setText(directory);
            }
        }

        var = d->configureComboBox->itemData(index, Qt::UserRole + 2);
        if (var.isValid()) {
            DetailPropertyWidget *detail = var.value<DetailPropertyWidget *>();
            if (detail && d->stackWidget) {
                d->stackWidget->setCurrentWidget(detail);
            }
        }

        ProjectConfigure *param = ConfigUtil::instance()->getConfigureParamPointer();
        param->tempSelType = ConfigUtil::instance()->getTypeFromName(d->configureComboBox->currentText());
        ConfigUtil::instance()->checkConfigInfo(d->configureComboBox->currentText(), d->outputDirEdit->text());
    });

    QHBoxLayout *hLayout = new QHBoxLayout();
    d->outputDirEdit = new DLineEdit(this);
    d->outputDirEdit->lineEdit()->setReadOnly(true);
    auto button = new QPushButton(this);
    button->setText(tr("Browse"));
    connect(button, &QPushButton::clicked, [this](){
        QString outputDirectory = QFileDialog::getExistingDirectory(this, "Output directory", d->outputDirEdit->text());
        if (!outputDirectory.isEmpty()) {
            QString oldDir = d->outputDirEdit->text();
            d->outputDirEdit->setText(outputDirectory.toUtf8());
            int index = d->configureComboBox->currentIndex();
            d->configureComboBox->setItemData(index, QVariant::fromValue(outputDirectory.toUtf8()), Qt::UserRole + 1);

            if (outputDirectory != oldDir) {
                ConfigUtil::instance()->checkConfigInfo(d->configureComboBox->currentText(), d->outputDirEdit->text());
            }
        }
    });

    hLayout->addWidget(d->outputDirEdit);
    hLayout->addWidget(button);
    hLayout->setSpacing(10);

    overviewLayout->setSpacing(0);
    overviewLayout->setMargin(0);
    overviewLayout->setSpacing(5);

    auto formlayout = new QFormLayout(this);
    formlayout->setContentsMargins(0, 0, 0, 0);
    formlayout->addRow(QLabel::tr("Build configuration:"), configureLayout);
    formlayout->addRow(tr("Output direcotry:"), hLayout);

    overviewLayout->addLayout(formlayout);
    buildCfgWidget->addWidget(overviewWidget);
    d->stackWidget = new DStackedWidget(this);
    buildCfgWidget->addWidget(d->stackWidget);
}

void BuildPropertyPage::initData(const dpfservice::ProjectInfo &projectInfo)
{
    d->configureComboBox->clear();

    ProjectConfigure *param = ConfigUtil::instance()->getConfigureParamPointer();
    auto iter = param->buildTypeConfigures.begin();
    int index = 0;
    for (; iter != param->buildTypeConfigures.end(); ++iter, index++) {
        for (auto iterStep = iter->buildConfigure.steps.begin(); iterStep != iter->buildConfigure.steps.end(); ++iterStep) {
            if (iterStep->allTargetNames.isEmpty()) {
                iterStep->allTargetNames = TargetsManager::instance()->getTargetNamesList();
                dpfservice::TargetType targetType = dpfservice::TargetType::kUnknown;
                if (iterStep->type == Build) {
                    targetType = dpfservice::TargetType::kBuildTarget;
                } else if (iterStep->type == Clean) {
                    targetType = dpfservice::TargetType::kCleanTarget;
                }
                dpfservice::Target target = TargetsManager::instance()->getActivedTargetByTargetType(targetType);
                iterStep->activeTargetName = target.buildTarget;
            }
        }

        DetailPropertyWidget *detailWidget = new DetailPropertyWidget(this);
        detailWidget->setValues(*iter);
        d->stackWidget->insertWidget(index, detailWidget);

        d->configureComboBox->insertItem(index, ConfigUtil::instance()->getNameFromType(iter->type));
        d->configureComboBox->setItemData(index, QVariant::fromValue(iter->directory), Qt::UserRole + 1);
        d->configureComboBox->setItemData(index, QVariant::fromValue(detailWidget), Qt::UserRole + 2);
        if (param->defaultType == iter->type) {
            d->configureComboBox->setCurrentIndex(index);
            d->outputDirEdit->setText(iter->directory);
            d->stackWidget->setCurrentWidget(detailWidget);
        }
    }
}

void BuildPropertyPage::updateDetail()
{
    ProjectConfigure *param = ConfigUtil::instance()->getConfigureParamPointer();

    QString currentTypeString = d->configureComboBox->currentText();
    auto iter = param->buildTypeConfigures.begin();
    for (; iter != param->buildTypeConfigures.end(); ++iter) {
        if (iter->type == ConfigUtil::instance()->getTypeFromName(currentTypeString)) {
            for (auto iterStep = iter->buildConfigure.steps.begin(); iterStep != iter->buildConfigure.steps.end(); ++iterStep) {
                iterStep->allTargetNames = TargetsManager::instance()->getTargetNamesList();
                dpfservice::Target target = TargetsManager::instance()->getActivedTargetByTargetType(d->typeMap.value(iterStep->type));
                iterStep->activeTargetName = target.buildTarget;
            }

            DetailPropertyWidget *detail = dynamic_cast<DetailPropertyWidget *>(d->stackWidget->currentWidget());
            if (detail) {
                detail->setValues(*iter);
            }
            break;
        }
    }
}

void BuildPropertyPage::readConfig()
{

}

void BuildPropertyPage::saveConfig()
{
    ProjectConfigure *param = ConfigUtil::instance()->getConfigureParamPointer();
    auto iter = param->buildTypeConfigures.begin();
    int index = 0;
    for (; iter != param->buildTypeConfigures.end(); ++iter) {
        DetailPropertyWidget *detailWidget = dynamic_cast<DetailPropertyWidget *>(d->stackWidget->widget(index));
        if (detailWidget) {
            detailWidget->getValues(*iter);
        }
        if (iter->type == param->tempSelType) {
            auto projectInfo = dpfGetService(ProjectService)->getActiveProjectInfo();
            auto activeTarget = TargetsManager::instance()->getActivedTargetByTargetType(kActiveExecTarget);
            for (auto it : iter->runConfigure.targetsRunConfigure) {
                if (it.targetName == activeTarget.name) {
                    projectInfo.setRunEnvironment(it.env.toList());
                    dpfGetService(ProjectService)->updateProjectInfo(projectInfo);
                }
            }
        }

        for (int i = 0; i < d->configureComboBox->count(); i++) {
            ConfigType type = ConfigUtil::instance()->getTypeFromName(d->configureComboBox->itemText(i));
            if (type == iter->type) {
                QVariant var = d->configureComboBox->itemData(index, Qt::UserRole + 1);
                if (var.isValid()) {
                    iter->directory = var.value<QString>();
                }

                if (d->configureComboBox->currentIndex() == i)
                    param->defaultType = type;

                break;
            }
        }
        index++;
    }
}
