// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configwidget.h"
#include "services/project/projectservice.h"
#include "common/util/custompaths.h"

#include <DLabel>
#include <DComboBox>
#include <DPushButton>
#include <DSuggestButton>

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

static const char *kConfigFileName = "codeporting.cfg";
static const char *kSrcCPU = "Src CPU";
static const char *kTargetCPU = "Target CPU";

// TODO(mozart):These value should get from backend later.
static const char *kX86_64 = "x86_64";
static const char *kArm = "arm64";
static const char *kMips = "mips64el";
static const char *kSW_64 = "sw_64";
static const char *kLoongarch64 = "loongarch64";

static const char *kSupportKit [] = {"cmake"};

using namespace dpfservice;

struct ConfigParameter
{
    QString project;
    QString srcCPU;
    QString targetCPU;
    QString reserve;
};

class ConfigWidgetPrivate
{
    friend ConfigWidget;
    QVBoxLayout *verticalLayout = nullptr;
    QGridLayout *gridLayout = nullptr;
    DLabel *lbProject = nullptr;
    DComboBox *combProject = nullptr;
    DLabel *lbSrc = nullptr;
    DComboBox *combSrc = nullptr;
    DLabel *lbTarget = nullptr;
    DComboBox *combTarget = nullptr;
    QHBoxLayout *horizontalLayout = nullptr;
    DPushButton *btnPoting = nullptr;
    DPushButton *btnCancel = nullptr;
    DLabel *lbWarning = nullptr;

    ConfigParameter cfgParam;
};

ConfigWidget::ConfigWidget(QWidget *parent)
    : DDialog(parent)
    , d(new ConfigWidgetPrivate)
{
    initializeUi();

    if (!restore()) {
        setDefaultValue();
    }

    connect(d->btnPoting, &DPushButton::clicked, this, &ConfigWidget::configDone);
    connect(d->btnCancel, &DPushButton::clicked, this, &ConfigWidget::reject);
}

void ConfigWidget::configDone()
{
    d->cfgParam.project = d->combProject->currentText();
    d->cfgParam.srcCPU = d->combSrc->currentText();
    d->cfgParam.targetCPU = d->combTarget->currentText();

    // empty parameter check.
    if (d->cfgParam.project.isEmpty()
            || d->cfgParam.srcCPU.isEmpty()
            || d->cfgParam.targetCPU.isEmpty()) {
        d->lbWarning->setText(tr("Warning: parameter is empty!"));
        return;
    }

    // repeat paramter check.
    if (d->cfgParam.srcCPU == d->cfgParam.targetCPU) {
        d->lbWarning->setText(tr("Warning: source cpu and target cpu should not be the same!"));
        return;
    }

    saveCfg();

    accept();

    d->lbWarning->clear();

    emit sigStartPorting(d->cfgParam.project, d->cfgParam.srcCPU, d->cfgParam.targetCPU);
}

void ConfigWidget::showEvent(QShowEvent *e)
{
    refreshUi();

    DDialog::showEvent(e);
}

void ConfigWidget::initializeUi()
{    
    setWindowTitle(tr("CodePorting config"));

    d->verticalLayout = static_cast<QVBoxLayout*>(this->layout());
    d->verticalLayout->setContentsMargins(10, 0, 10, 0);

    d->gridLayout = new QGridLayout();
    d->gridLayout->setSpacing(6);

    d->lbProject = new DLabel(this);
    d->lbProject->setText(tr("Project:"));
    d->gridLayout->addWidget(d->lbProject, 0, 0, 1, 1);

    d->combProject = new DComboBox(this);
    d->gridLayout->addWidget(d->combProject, 0, 1, 1, 1);

    d->lbSrc = new DLabel(this);
    d->lbSrc->setText(tr("Source CPU Architecture:"));
    d->gridLayout->addWidget(d->lbSrc, 1, 0, 1, 1);

    d->combSrc = new DComboBox(this);
    d->gridLayout->addWidget(d->combSrc, 1, 1, 1, 1);

    d->lbTarget = new DLabel(this);
    d->lbTarget->setText(tr("Target CPU Architecture:"));
    d->gridLayout->addWidget(d->lbTarget, 2, 0, 1, 1);

    d->combTarget = new DComboBox(this);
    d->gridLayout->addWidget(d->combTarget, 2, 1, 1, 1);
    d->verticalLayout->addLayout(d->gridLayout);

    d->lbWarning = new DLabel(this);
    QPalette pe;
    pe.setColor(QPalette::Text, Qt::yellow);
    d->lbWarning->setPalette(pe);
    d->verticalLayout->addWidget(d->lbWarning);

    d->horizontalLayout = new QHBoxLayout();
    d->horizontalLayout->setContentsMargins(0, 0, 0, 10);
    d->btnCancel = new DPushButton(this);
    d->btnCancel->setText(tr("Cancel"));
    d->horizontalLayout->addWidget(d->btnCancel);
    d->verticalLayout->addLayout(d->horizontalLayout);

    d->btnPoting = new DSuggestButton(this);
    d->btnPoting->setText(tr("Porting"));
    d->btnPoting->setDefault(true);
    d->horizontalLayout->addSpacing(20);
    d->horizontalLayout->addWidget(d->btnPoting);

    d->combSrc->addItem(kX86_64);
    d->combSrc->addItem(kArm);
    d->combSrc->addItem(kMips);
    d->combSrc->addItem(kSW_64);
    d->combSrc->addItem(kLoongarch64);

    d->combTarget->addItem(kX86_64);
    d->combTarget->addItem(kArm);
    d->combTarget->addItem(kMips);
    d->combTarget->addItem(kSW_64);
    d->combTarget->addItem(kLoongarch64);

    setFixedSize(QSize(380, 254));
}

void ConfigWidget::setDefaultValue()
{
    d->cfgParam.srcCPU = "x86_64";
    d->cfgParam.targetCPU = "arm64";
}

void ConfigWidget::resetUi()
{
    int emptyIndex = -1;
    d->combProject->setCurrentIndex(emptyIndex);
    d->combSrc->setCurrentIndex(emptyIndex);
    d->combTarget->setCurrentIndex(emptyIndex);
}

void ConfigWidget::refreshDetail()
{
    d->combSrc->setCurrentText(d->cfgParam.srcCPU);
    d->combTarget->setCurrentText(d->cfgParam.targetCPU);
}

bool ConfigWidget::saveCfg()
{
    QString cfgFilePath = configFilePath();

    QJsonArray paramsArray;

    QJsonObject valueObj;
    valueObj.insert(kSrcCPU, d->cfgParam.srcCPU);
    valueObj.insert(kTargetCPU, d->cfgParam.targetCPU);

    paramsArray.append(valueObj);

    QJsonObject rootObject;
    rootObject.insert("Configure", paramsArray);
    QJsonDocument doc;
    doc.setObject(rootObject);
    QString jsonStr(doc.toJson(QJsonDocument::Indented));

    QFile file(cfgFilePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(jsonStr.toUtf8());
    file.close();

    return true;
}

bool ConfigWidget::restore()
{
    QString cfgFilePath = configFilePath();

    QFile file(cfgFilePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (QJsonParseError::NoError != parseError.error) {
        return false;
    }

    if (!doc.isObject())
        return false;

    QJsonObject rootObject = doc.object();
    for (auto key : rootObject.keys()) {
        if (!rootObject.value(key).isArray() || !rootObject.keys().contains("Configure"))
            continue;

        if (rootObject.keys().contains("Configure")) {
            QJsonArray valueArray = rootObject.value("Configure").toArray();

            for (QJsonValue value : valueArray) {
                QJsonObject obj = value.toObject();
                d->cfgParam.srcCPU = obj.value(kSrcCPU).toString();
                d->cfgParam.targetCPU = obj.value(kTargetCPU).toString();
            }
        }
    }
    return true;
}

QString ConfigWidget::configFilePath()
{
    return CustomPaths::endSeparator(CustomPaths::user(CustomPaths::Configures)) + kConfigFileName;
}

void ConfigWidget::refreshUi()
{
    auto &ctx = dpfInstance.serviceContext();
    ProjectService *projectService = ctx.service<ProjectService>(ProjectService::name());
    d->combProject->clear();
    if (projectService && projectService->getAllProjectInfo) {
        auto allInfo = projectService->getAllProjectInfo();
        if (allInfo.isEmpty()) {
            resetUi();
            return;
        } else {
            for (auto projInfo : allInfo) {
                QString path = projInfo.workspaceFolder();
                QString dirName = path.split("/").back();
                QString kitName = projInfo.kitName();
                int supportkitCount = sizeof(kSupportKit)/sizeof(kSupportKit[0]);
                for (int i = 0; i < supportkitCount; ++i) {
                    if (kSupportKit[i] == projInfo.kitName()) {
                        d->combProject->addItem(dirName);
                        break;
                    }
                }
            }
        }
    }
    refreshDetail();
}
