/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "configwidget.h"
#include "services/project/projectservice.h"
#include "common/util/custompaths.h"

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
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
    QLabel *lbProject = nullptr;
    QComboBox *combProject = nullptr;
    QLabel *lbSrc = nullptr;
    QComboBox *combSrc = nullptr;
    QLabel *lbTarget = nullptr;
    QComboBox *combTarget = nullptr;
    QSpacerItem *verticalSpacer = nullptr;
    QHBoxLayout *horizontalLayout = nullptr;
    QSpacerItem *horizontalSpacer = nullptr;
    QPushButton *btnPoting = nullptr;
    QPushButton *btnCancel = nullptr;
    QLabel *lbWarning = nullptr;

    ConfigParameter cfgParam;
};

ConfigWidget::ConfigWidget(QWidget *parent)
    : QDialog(parent)
    , d(new ConfigWidgetPrivate)
{
    setupUi(this);

    initializeUi();

    if (!restore()) {
        setDefaultValue();
    }

    connect(d->btnPoting, &QPushButton::clicked, this, &ConfigWidget::configDone);
    connect(d->btnCancel, &QPushButton::clicked, this, &ConfigWidget::reject);
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

    QDialog::showEvent(e);
}

void ConfigWidget::initializeUi()
{
    d->combSrc->addItem(kX86_64);
    d->combSrc->addItem(kArm);
    d->combSrc->addItem(kMips);
    d->combSrc->addItem(kSW_64);

    d->combTarget->addItem(kX86_64);
    d->combTarget->addItem(kArm);
    d->combTarget->addItem(kMips);
    d->combTarget->addItem(kSW_64);
}

void ConfigWidget::setDefaultValue()
{
    d->cfgParam.srcCPU = "x86_64";
    d->cfgParam.targetCPU = "arm64";
}

void ConfigWidget::setupUi(QWidget *Widget)
{
    Widget->setWindowTitle(tr("CodePorting config"));
    Widget->resize(500, 200);
    d->verticalLayout = new QVBoxLayout(Widget);
    d->verticalLayout->setSpacing(6);
    d->verticalLayout->setContentsMargins(11, 11, 11, 11);
    d->verticalLayout->setContentsMargins(40, 20, 40, 20);
    d->gridLayout = new QGridLayout();
    d->gridLayout->setSpacing(6);
    d->lbProject = new QLabel(Widget);
    d->lbProject->setText(tr("Project:"));

    d->gridLayout->addWidget(d->lbProject, 0, 0, 1, 1);

    d->combProject = new QComboBox(Widget);

    d->gridLayout->addWidget(d->combProject, 0, 1, 1, 1);

    d->lbSrc = new QLabel(Widget);
    d->lbSrc->setText(tr("Source CPU Architecture:"));

    d->gridLayout->addWidget(d->lbSrc, 1, 0, 1, 1);

    d->combSrc = new QComboBox(Widget);

    d->gridLayout->addWidget(d->combSrc, 1, 1, 1, 1);

    d->lbTarget = new QLabel(Widget);
    d->lbTarget->setText(tr("Target CPU Architecture:"));

    d->gridLayout->addWidget(d->lbTarget, 2, 0, 1, 1);

    d->combTarget = new QComboBox(Widget);

    d->gridLayout->addWidget(d->combTarget, 2, 1, 1, 1);

    d->verticalLayout->addLayout(d->gridLayout);

    d->lbWarning = new QLabel(Widget);
    QPalette pe;
    pe.setColor(QPalette::Text, Qt::yellow);
    d->lbWarning->setPalette(pe);

    d->verticalLayout->addWidget(d->lbWarning);

    d->verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    d->verticalLayout->addItem(d->verticalSpacer);

    d->horizontalLayout = new QHBoxLayout();
    d->horizontalSpacer = new QSpacerItem(40, 30, QSizePolicy::Expanding, QSizePolicy::Minimum);

    d->horizontalLayout->addItem(d->horizontalSpacer);

    d->btnPoting = new QPushButton(Widget);
    d->btnPoting->setText(tr("Porting"));
    d->btnPoting->setDefault(true);

    d->horizontalLayout->addWidget(d->btnPoting);

    d->btnCancel = new QPushButton(Widget);
    d->btnCancel->setText(tr("Cancel"));

    d->horizontalLayout->addWidget(d->btnCancel);

    d->verticalLayout->addLayout(d->horizontalLayout);
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
    if (projectService && projectService->projectView.getAllProjectInfo) {
        auto allInfo = projectService->projectView.getAllProjectInfo();
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
