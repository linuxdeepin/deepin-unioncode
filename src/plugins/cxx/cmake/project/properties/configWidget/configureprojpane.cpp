// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configureprojpane.h"

#include "cmakeasynparse.h"
#include "cmakeprojectgenerator.h"

#include "services/option/optionmanager.h"

#include <DCheckBox>
#include <DLineEdit>
#include <DPushButton>
#include <DWidget>
#include <DRadioButton>
#include <DComboBox>
#include <DFileDialog>

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QUrl>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QButtonGroup>

// TODO(mozart):should get from kitmanager.
static const char *kDefaultKitName = "Desktop";

DWIDGET_USE_NAMESPACE
using namespace config;

class ConfigureProjPanePrivate
{
    friend class ConfigureProjPane;

    DComboBox *kitComboBox{nullptr};

    DRadioButton *radioDebug{nullptr};
    DRadioButton *radioRelease{nullptr};

    DLineEdit *lineEditDebug{nullptr};
    DLineEdit *lineEditRelease{nullptr};

    DPushButton *btnRWithDInfo{nullptr};

    QButtonGroup *group{nullptr};

    ConfigureParam *cfgItem{nullptr};
};

ConfigureProjPane::ConfigureProjPane(const QString &language,
                                     const QString &workspace,
                                     QWidget *parent)
    : DWidget(parent)
    , d(new ConfigureProjPanePrivate)
{
    d->cfgItem = ConfigUtil::instance()->getConfigureParamPointer();
    d->cfgItem->clear();
    d->cfgItem->language = language;
    d->cfgItem->workspace = workspace;

    setupUI();
    updateUI();

    connect(ConfigUtil::instance(), QOverload<const dpfservice::ProjectInfo &>::of(&ConfigUtil::configureDone),
            [this](const dpfservice::ProjectInfo &info) {
        QString propertyPath = ConfigUtil::instance()->getConfigPath(d->cfgItem->workspace);
        config::ConfigUtil::instance()->saveConfig(propertyPath, *d->cfgItem);
        dpfservice::ProjectInfo projectInfo = info;
        config::ConfigUtil::instance()->updateProjectInfo(projectInfo, d->cfgItem);
        emit configureDone(projectInfo);
    });
}

ConfigureProjPane::~ConfigureProjPane()
{
    if (d)
        delete d;
}

void ConfigureProjPane::setupUI()
{
    auto btnSignalConnect = [this](DPushButton *btn, DLineEdit *lineEdit){
        connect(btn, &DPushButton::clicked, [=](){
            QString outputDirectory = DFileDialog::getExistingDirectory(this, "Output directory");
            if (!outputDirectory.isEmpty()) {
                lineEdit->setText(outputDirectory);
            }
        });
    };
    DLabel *kitLabel = new DLabel(tr("Select kit: "));
    kitLabel->setFixedWidth(100);
    d->kitComboBox = new DComboBox(this);
    d->kitComboBox->addItem(kDefaultKitName);
    d->kitComboBox->setCurrentIndex(0);
    QHBoxLayout *hLayoutKit = new QHBoxLayout();
    hLayoutKit->addWidget(kitLabel, 0, Qt::AlignLeft);
    hLayoutKit->addWidget(d->kitComboBox, 0, Qt::AlignLeft);
    hLayoutKit->addStretch(10);

    d->radioDebug = new DRadioButton("Debug");
    d->radioDebug->setFixedWidth(100);
    auto btnDebug = new DPushButton(tr("Browse"));
    d->lineEditDebug = new DLineEdit(this);
    d->lineEditDebug->setMinimumWidth(280);
    btnSignalConnect(btnDebug, d->lineEditDebug);
    QHBoxLayout *hLayoutDebug = new QHBoxLayout();
    hLayoutDebug->addWidget(d->radioDebug);
    hLayoutDebug->addWidget(d->lineEditDebug);
    hLayoutDebug->addWidget(btnDebug);

    d->radioRelease = new DRadioButton("Release");
    d->radioRelease->setFixedWidth(100);
    auto btnRelease = new DPushButton(tr("Browse"));
    d->lineEditRelease = new DLineEdit(this);
    d->lineEditDebug->setMinimumWidth(280);
    btnSignalConnect(btnRelease, d->lineEditRelease);
    QHBoxLayout *hLayoutRelease = new QHBoxLayout();
    hLayoutRelease->addWidget(d->radioRelease);
    hLayoutRelease->addWidget(d->lineEditRelease);
    hLayoutRelease->addWidget(btnRelease);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addLayout(hLayoutKit);
    vLayout->addLayout(hLayoutDebug);
    vLayout->addLayout(hLayoutRelease);
    setLayout(vLayout);

    d->group = new QButtonGroup(this);
    d->group->addButton(d->radioDebug, 0);
    d->group->addButton(d->radioRelease, 1);
    d->radioDebug->setChecked(true);
}

void ConfigureProjPane::resetUI()
{
    d->kitComboBox->clear();
    d->radioDebug->setChecked(true);
    d->lineEditDebug->clear();
    d->lineEditRelease->clear();
}

void ConfigureProjPane::updateUI()
{
    resetUI();

    d->kitComboBox->addItem(kDefaultKitName);

    if (d->cfgItem->workspace.isEmpty())
        return;

    QDir folder(d->cfgItem->workspace);
    QString folderName = folder.dirName();
    folder.cdUp();
    QString upDirectory = folder.path();

    auto folerPath = [=](QString buildType) -> QString {
        return (upDirectory + QDir::separator() + "build" + "-" + folderName + "-" + kDefaultKitName + "-" + buildType);
    };

    d->lineEditDebug->setText(folerPath(ConfigUtil::instance()->getNameFromType(Debug)));
    d->lineEditRelease->setText(folerPath(ConfigUtil::instance()->getNameFromType(Release)));
}

void ConfigureProjPane::slotConfigure()
{
    QList<ConfigType> initType = {Debug, Release};
    foreach (auto type, initType) {
        BuildConfigure buildConfigure;
        buildConfigure.type = type;
        // init output directory
        if (Debug == type) {
            buildConfigure.directory = d->lineEditDebug->text().trimmed();
        } else if (Release == type) {
            buildConfigure.directory = d->lineEditRelease->text().trimmed();
        }
        // init build steps
        for (int i = 0; i < StepCount; i++) {
            StepItem item;
            item.type = static_cast<StepType>(i);
            // default use max thread count
            int coreCount = QThread::idealThreadCount();
            if (coreCount > 1) {
                item.arguments = QString("-j%1").arg(coreCount);
            }
            buildConfigure.steps.push_back(item);
        }
        // init environment
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        foreach (auto key, env.keys()) {
            buildConfigure.env.environments.insert(key, env.value(key));
        }

        d->cfgItem->buildConfigures.push_back(buildConfigure);
    }

    d->cfgItem->defaultType = ConfigUtil::instance()->getTypeFromName(d->group->checkedButton()->text());
    d->cfgItem->tempSelType = d->cfgItem->defaultType;
    d->cfgItem->kit = d->kitComboBox->currentText();

    ConfigUtil::instance()->configProject(d->cfgItem);
}
