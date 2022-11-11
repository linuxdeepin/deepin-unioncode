/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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
#include "configureprojpane.h"

#include "mainframe/cmakeasynparse.h"
#include "mainframe/cmakegenerator.h"

#include "services/option/optionmanager.h"

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <QtWidgets/QSpacerItem>
#include <QFileDialog>
#include <QUrl>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>

// TODO(mozart):should get from kitmanager.
static const char *kDefaultKitName = "Desktop";
static const char *kConfigFileName = "configure.support";

using namespace config;

class ConfigureProjPanePrivate
{
    friend class ConfigureProjPane;

    QComboBox *kitComboBox{nullptr};

    QRadioButton *radioDebug{nullptr};
    QRadioButton *radioRelease{nullptr};

    QLineEdit *lineEditDebug{nullptr};
    QLineEdit *lineEditRelease{nullptr};

    QPushButton *btnRWithDInfo{nullptr};
    QPushButton *btnConfigure{nullptr};

    QButtonGroup *group{nullptr};

    ConfigureParam *cfgItem{nullptr};

    QString projectPath;
    QString language;
};

ConfigureProjPane::ConfigureProjPane(const QString &language,
                                     const QString &projectPath,
                                     QWidget *parent)
    : QWidget(parent)
    , d(new ConfigureProjPanePrivate)
{
    d->projectPath = projectPath;
    d->language = language;
    d->cfgItem = ConfigUtil::instance()->getConfigureParamPointer();

    setupUI();
    updateUI();
}

ConfigureProjPane::~ConfigureProjPane()
{
    if (d)
        delete d;
}

void ConfigureProjPane::setupUI()
{
    auto btnSignalConnect = [this](QPushButton *btn, QLineEdit *lineEdit){
        connect(btn, &QPushButton::clicked, [=](){
            QString outputDirectory = QFileDialog::getExistingDirectory(this, "Output directory");
            if (!outputDirectory.isEmpty()) {
                lineEdit->setText(outputDirectory);
            }
        });
    };

    QLabel *kitLabel = new QLabel(tr("Select kit: "));
    kitLabel->setFixedWidth(100);
    d->kitComboBox = new QComboBox();
    d->kitComboBox->addItem(kDefaultKitName);
    d->kitComboBox->setCurrentIndex(0);
    QHBoxLayout *hLayoutKit = new QHBoxLayout();
    hLayoutKit->addWidget(kitLabel, 0, Qt::AlignLeft);
    hLayoutKit->addWidget(d->kitComboBox, 0, Qt::AlignLeft);
    hLayoutKit->addStretch(10);

    d->radioDebug = new QRadioButton(tr("Debug"));
    d->radioDebug->setFixedWidth(100);
    auto btnDebug = new QPushButton(tr("Browse..."));
    d->lineEditDebug = new QLineEdit();
    d->lineEditDebug->setMinimumWidth(400);
    btnSignalConnect(btnDebug, d->lineEditDebug);
    QHBoxLayout *hLayoutDebug = new QHBoxLayout();
    hLayoutDebug->addWidget(d->radioDebug);
    hLayoutDebug->addWidget(d->lineEditDebug);
    hLayoutDebug->addWidget(btnDebug);

    d->radioRelease = new QRadioButton(tr("Release"));
    d->radioRelease->setFixedWidth(100);
    auto btnRelease = new QPushButton(tr("Browse..."));
    d->lineEditRelease = new QLineEdit();
    d->lineEditDebug->setMinimumWidth(400);
    btnSignalConnect(btnRelease, d->lineEditRelease);
    QHBoxLayout *hLayoutRelease = new QHBoxLayout();
    hLayoutRelease->addWidget(d->radioRelease);
    hLayoutRelease->addWidget(d->lineEditRelease);
    hLayoutRelease->addWidget(btnRelease);

    auto btnConfigure = new QPushButton(tr("Configure"));
    btnConfigure->connect(btnConfigure, &QPushButton::clicked,
                          this, &ConfigureProjPane::slotConfigureDone);

    QHBoxLayout *hLayoutBottom = new QHBoxLayout();
    hLayoutBottom->addStretch(10);
    hLayoutBottom->addWidget(btnConfigure, 0, Qt::AlignRight);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addLayout(hLayoutKit);
    vLayout->addSpacing(10);
    vLayout->addLayout(hLayoutDebug);
    vLayout->addLayout(hLayoutRelease);
    vLayout->addSpacing(10);
    vLayout->addLayout(hLayoutBottom);
    setLayout(vLayout);

    d->group = new QButtonGroup();
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

    if (d->projectPath.isEmpty())
        return;

    QDir folder = QFileInfo(d->projectPath).dir();
    QString folderName = folder.dirName();
    folder.cdUp();
    QString upDirectory = folder.path();

    auto folerPath = [&](QString buildType) -> QString {
        return (upDirectory + QDir::separator() + "build" + "-" + folderName + "-" + kDefaultKitName + "-" + buildType);
    };

    d->lineEditDebug->setText(folerPath(ConfigUtil::instance()->getNameFromType(Debug)));
    d->lineEditRelease->setText(folerPath(ConfigUtil::instance()->getNameFromType(Release)));
}

void ConfigureProjPane::slotConfigureDone()
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
    d->cfgItem->kit = d->kitComboBox->currentText();
    d->cfgItem->language = d->language;
    d->cfgItem->projectPath = d->projectPath;

    configProject(d->cfgItem);
}

void ConfigureProjPane::configProject(const ConfigureParam *param)
{
    dpfservice::ProjectInfo info;
    QString sourceFolder = QFileInfo(d->projectPath).path();
    info.setLanguage(d->language);
    info.setSourceFolder(sourceFolder);
    info.setKitName(CmakeGenerator::toolKitName());
    //    info.setBuildFolder(outputPath);
    info.setWorkspaceFolder(sourceFolder);
    info.setProjectFilePath(d->projectPath);

    for (auto iter = param->buildConfigures.begin(); iter != param->buildConfigures.end(); ++iter) {
        if (param->defaultType == iter->type) {

            info.setBuildType(ConfigUtil::instance()->getNameFromType(param->defaultType));
            info.setBuildFolder(iter->directory);
            info.setBuildProgram(OptionManager::getInstance()->getCMakeToolPath());

            QStringList arguments;
            arguments << "-S";
            arguments << info.sourceFolder();
            arguments << "-B";
            arguments << info.buildFolder();
            arguments << "-G";
            arguments << CDT_PROJECT_KIT::get()->CDT4_GENERATOR;
            arguments << "-DCMAKE_BUILD_TYPE=" + info.buildType();
            arguments << "-DCMAKE_EXPORT_COMPILE_COMMANDS=1";
            arguments << info.buildCustomArgs();
            info.setBuildCustomArgs(arguments);

            break;
        }
    }

    emit configureDone(info);
}
