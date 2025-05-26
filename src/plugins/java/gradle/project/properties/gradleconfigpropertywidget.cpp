// SPDX-FileCopyrightText: 2023 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gradleconfigpropertywidget.h"

#include "services/option/toolchaindata.h"
#include "common/toolchain/toolchain.h"
#include "common/widget/pagewidget.h"

#include <DComboBox>
#include <DLabel>
#include <DLineEdit>
#include <DCheckBox>
#include <DPushButton>
#include <DFileDialog>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

static const QString kJrePath = QObject::tr("jre path");
static const QString kJreExecute = QObject::tr("jre execute");
static const QString kLaunchConfigPath = QObject::tr("launch config");
static const QString kLaunchPackageFile = QObject::tr("launch package");
static const QString kDapPackageFile = QObject::tr("dap package");
static const int kHeadlineWidth = 120;

class GradleDetailPropertyWidgetPrivate
{
    friend class GradleDetailPropertyWidget;
    DComboBox *jdkVersionComboBox{nullptr};
    DComboBox *gradleVersionComboBox{nullptr};
    DLineEdit *mainClass{nullptr};
    QSharedPointer<ToolChainData> toolChainData;
    DCheckBox *detailBox{nullptr};
    DLineEdit *jreEdit{nullptr};
    DLineEdit *jreExecuteEdit{nullptr};
    DLineEdit *launchCfgPathEdit{nullptr};
    DLineEdit *lanuchCfgFileEdit{nullptr};
    DLineEdit *dapPackageFileEdit{nullptr};
};

GradleDetailPropertyWidget::GradleDetailPropertyWidget(QWidget *parent)
    : DFrame(parent)
    , d(new GradleDetailPropertyWidgetPrivate())
{
    setupUI();
    initData();
}

GradleDetailPropertyWidget::~GradleDetailPropertyWidget()
{
    if (d)
        delete d;
}

void GradleDetailPropertyWidget::setupUI()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    QHBoxLayout *hLayout = new QHBoxLayout();
    DLabel *label = new DLabel(QLabel::tr("JDK version："));
    label->setFixedWidth(kHeadlineWidth);
    d->jdkVersionComboBox = new DComboBox();
    hLayout->addWidget(label);
    hLayout->addWidget(d->jdkVersionComboBox);
    vLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout();
    label = new DLabel(QLabel::tr("Gradle Version: "));
    label->setFixedWidth(kHeadlineWidth);
    d->gradleVersionComboBox = new DComboBox();
    hLayout->addWidget(label);
    hLayout->addWidget(d->gradleVersionComboBox);
    vLayout->addLayout(hLayout);

    hLayout = new QHBoxLayout();
    label = new DLabel(QLabel::tr("Main Class："));
    label->setFixedWidth(kHeadlineWidth);
    d->mainClass = new DLineEdit();
    d->mainClass->setPlaceholderText(tr("Input main class"));
    hLayout->addWidget(label);
    hLayout->addWidget(d->mainClass);
    vLayout->addLayout(hLayout);
    vLayout->addStretch(10);

    hLayout = new QHBoxLayout();
    label = new DLabel(QLabel::tr("Detail output:"));
    label->setFixedWidth(kHeadlineWidth);
    d->detailBox = new DCheckBox();
    hLayout->addWidget(label);
    hLayout->addWidget(d->detailBox);
    hLayout->setAlignment(Qt::AlignLeft);
    vLayout->addLayout(hLayout);

    // Dap plugins config.
    auto addGroupWidgets = [this](QVBoxLayout *vLayout, const QString &headLine, QWidget *widget){

        QHBoxLayout *hLayout = new QHBoxLayout();
        DLabel *label = new DLabel(headLine + ":");
        label->setFixedWidth(kHeadlineWidth);

        DPushButton *btnBrowser = new DPushButton(this);
        btnBrowser->setText(tr("Browse"));
        btnBrowser->setObjectName(headLine);
        hLayout->addWidget(label);
        hLayout->addWidget(widget);
        hLayout->addWidget(btnBrowser);
        vLayout->addLayout(hLayout);

        connect(btnBrowser, &DPushButton::clicked, this, &GradleDetailPropertyWidget::browserFileDialog);
    };

    d->jreEdit = new DLineEdit(this);
    d->jreExecuteEdit = new DLineEdit(this);
    d->launchCfgPathEdit = new DLineEdit(this);
    d->lanuchCfgFileEdit = new DLineEdit(this);
    d->dapPackageFileEdit = new DLineEdit(this);

    addGroupWidgets(vLayout, kJrePath, d->jreEdit);
    addGroupWidgets(vLayout, kJreExecute, d->jreExecuteEdit);
    addGroupWidgets(vLayout, kLaunchConfigPath, d->launchCfgPathEdit);
    addGroupWidgets(vLayout, kLaunchPackageFile, d->lanuchCfgFileEdit);
    addGroupWidgets(vLayout, kDapPackageFile,d->dapPackageFileEdit);
}

void GradleDetailPropertyWidget::initData()
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

        initComboBox(d->jdkVersionComboBox, data, kJDK);
        initComboBox(d->gradleVersionComboBox, data, kGradle);
    }
}

void GradleDetailPropertyWidget::setValues(const gradleConfig::ConfigureParam *param)
{
    if (!param)
        return;

    auto initComboBox = [](DComboBox *comboBox, const gradleConfig::ItemInfo &itemInfo) {
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
    d->jreEdit->setText(param->jrePath);
    d->jreExecuteEdit->setText(param->jreExecute);
    d->launchCfgPathEdit->setText(param->launchConfigPath);
    d->lanuchCfgFileEdit->setText(param->launchPackageFile);
    d->dapPackageFileEdit->setText(param->dapPackageFile);
}

void GradleDetailPropertyWidget::getValues(gradleConfig::ConfigureParam *param)
{
    if (!param)
        return;

    auto getValue = [](DComboBox *comboBox, gradleConfig::ItemInfo &itemInfo){
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
    param->jrePath = d->jreEdit->text();
    param->jreExecute = d->jreExecuteEdit->text();
    param->launchConfigPath = d->launchCfgPathEdit->text();
    param->launchPackageFile = d->lanuchCfgFileEdit->text();
    param->dapPackageFile = d->dapPackageFileEdit->text();
}

void GradleDetailPropertyWidget::browserFileDialog()
{
    QObject *senderObj = qobject_cast<QObject *>(sender());
    QString senderName = senderObj->objectName();

    auto showDirDialog = [this](DLineEdit *widget){
        QString result = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                           widget->text(),
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!result.isEmpty()) {
            widget->setText(result);
        }
    };

    auto showFileDialog = [this](DLineEdit *widget){
        QString result = QFileDialog::getOpenFileName(this, tr("Select File"), widget->text());
        if (!result.isEmpty())
            widget->setText(result);
    };

    if (senderName == kJrePath) {
        showDirDialog(d->jreEdit);
    } else if (senderName == kJreExecute) {
        showFileDialog(d->jreExecuteEdit);
    } else if (senderName == kLaunchConfigPath) {
        showDirDialog(d->launchCfgPathEdit);
    } else if (senderName == kLaunchPackageFile) {
        showFileDialog(d->lanuchCfgFileEdit);
    } else if (senderName == kDapPackageFile) {
        showFileDialog(d->dapPackageFileEdit);
    }
}

class GradleConfigPropertyWidgetPrivate
{
    friend class GradleConfigPropertyWidget;

    GradleDetailPropertyWidget *detail{nullptr};
    QStandardItem *item{nullptr};
    dpfservice::ProjectInfo projectInfo;
};

GradleConfigPropertyWidget::GradleConfigPropertyWidget(const dpfservice::ProjectInfo &projectInfo, QStandardItem *item, QWidget *parent)
    : PageWidget(parent)
    , d(new GradleConfigPropertyWidgetPrivate())
{
    d->item = item;
    d->projectInfo = projectInfo;
    setupUI();
    initData(projectInfo);
}

GradleConfigPropertyWidget::~GradleConfigPropertyWidget()
{
    if (d)
        delete d;
}

void GradleConfigPropertyWidget::setupUI()
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    setLayout(vLayout);

    d->detail = new GradleDetailPropertyWidget();
    vLayout->addWidget(d->detail);
    vLayout->addStretch(10);
}

void GradleConfigPropertyWidget::initData(const dpfservice::ProjectInfo &projectInfo)
{
    gradleConfig::ConfigureParam *param = gradleConfig::ConfigUtil::instance()->getConfigureParamPointer();
    d->detail->setValues(param);
    param->kit = projectInfo.kitName();
    param->language = projectInfo.language();
    param->projectPath = projectInfo.workspaceFolder();
}

void GradleConfigPropertyWidget::saveConfig()
{
    gradleConfig::ConfigureParam *param = gradleConfig::ConfigUtil::instance()->getConfigureParamPointer();
    d->detail->getValues(param);

    QString filePath = gradleConfig::ConfigUtil::instance()->getConfigPath(param->projectPath);
    gradleConfig::ConfigUtil::instance()->saveConfig(filePath, *param);

    gradleConfig::ConfigUtil::instance()->updateProjectInfo(d->projectInfo, param);
    dpfservice::ProjectInfo::set(d->item, d->projectInfo);
}
